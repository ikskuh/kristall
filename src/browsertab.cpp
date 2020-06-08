#include "browsertab.hpp"
#include "ui_browsertab.h"
#include "mainwindow.hpp"
#include "geminirenderer.hpp"
#include "settingsdialog.hpp"
#include "gophermaprenderer.hpp"

#include <QTabWidget>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>
#include <QImage>
#include <QPixmap>

#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>


BrowserTab::BrowserTab(MainWindow * mainWindow) :
    QWidget(nullptr),
    ui(new Ui::BrowserTab),
    mainWindow(mainWindow),
    outline(),
    graphics_scene()
{
    ui->setupUi(this);

    connect(&web_client, &WebClient::requestComplete, this, &BrowserTab::on_requestComplete);
    connect(&web_client, &WebClient::requestFailed, this, &BrowserTab::on_requestFailed);

    connect(&gemini_client, &GeminiClient::requestComplete, this, &BrowserTab::on_requestComplete);
    connect(&gemini_client, &GeminiClient::protocolViolation, this, &BrowserTab::on_protocolViolation);
    connect(&gemini_client, &GeminiClient::inputRequired, this, &BrowserTab::on_inputRequired);
    connect(&gemini_client, &GeminiClient::redirected, this, &BrowserTab::on_redirected);
    connect(&gemini_client, &GeminiClient::temporaryFailure, this, &BrowserTab::on_temporaryFailure);
    connect(&gemini_client, &GeminiClient::permanentFailure, this, &BrowserTab::on_permanentFailure);
    connect(&gemini_client, &GeminiClient::transientCertificateRequested, this, &BrowserTab::on_transientCertificateRequested);
    connect(&gemini_client, &GeminiClient::authorisedCertificateRequested, this, &BrowserTab::on_authorisedCertificateRequested);
    connect(&gemini_client, &GeminiClient::certificateRejected, this, &BrowserTab::on_certificateRejected);

    connect(&gopher_client, &GopherClient::requestComplete, this, &BrowserTab::on_requestComplete);
    connect(&gopher_client, &GopherClient::requestFailed, this, &BrowserTab::on_requestFailed);


    this->updateUI();

    this->ui->media_browser->setVisible(false);
    this->ui->graphics_browser->setVisible(false);
    this->ui->text_browser->setVisible(true);

    this->ui->graphics_browser->setScene(&graphics_scene);
}

BrowserTab::~BrowserTab()
{
    delete ui;
}

void BrowserTab::navigateTo(const QUrl &url, PushToHistory mode)
{
    if(not mainWindow->protocols.isSchemeSupported(url.scheme()))
    {
        QMessageBox::warning(this, "Kristall", "Unsupported uri scheme: " + url.scheme());
        return;
    }

    this->current_location = url;
    this->ui->url_bar->setText(url.toString());

    if(not gemini_client.cancelRequest()) {
        QMessageBox::warning(this, "Kristall", "Failed to cancel running gemini request!");
        return;
    }

    if(not web_client.cancelRequest()) {
        QMessageBox::warning(this, "Kristall", "Failed to cancel running web request!");
        return;
    }

    if(not gopher_client.cancelRequest()) {
        QMessageBox::warning(this, "Kristall", "Failed to cancel running gopher request!");
        return;
    }

    this->redirection_count = 0;
    this->successfully_loaded = false;
    this->push_to_history_after_load = (mode == PushAfterSuccess);

    if(url.scheme() == "gemini")
    {
        gemini_client.startRequest(url);
    }
    else if(url.scheme() == "http" or url.scheme() == "https")
    {
        web_client.startRequest(url);
    }
    else if(url.scheme() == "gopher")
    {
        gopher_client.startRequest(url);
    }
    else if(url.scheme() == "about")
    {
        this->redirection_count = 0;
        this->push_to_history_after_load = false;

        if(mode == PushAfterSuccess)
            mode = PushImmediate;
        if(url.path() == "blank")
        {
            this->on_requestComplete("", "text/gemini");
        }
        else if(url.path() == "favourites")
        {
            QByteArray document;

            document.append("# Favourites\n");
            document.append("\n");

            for(auto const & fav : this->mainWindow->favourites.getAll())
            {
                document.append("=> " + fav.toString().toUtf8() + "\n");
            }

            this->on_requestComplete(document, "text/gemini");
        }
        else
        {
            QMessageBox::warning(this, "Kristall", "Unknown location: " + url.path());
        }
    }


    switch(mode)
    {
    case DontPush:
    case PushAfterSuccess:
        break;

    case PushImmediate:
        pushToHistory(url);
        break;
    }

    this->updateUI();
}

void BrowserTab::navigateBack(QModelIndex history_index)
{
    auto url = history.get(history_index);

    if(url.isValid()) {
        current_history_index = history_index;
        navigateTo(url, DontPush);
    }
}

void BrowserTab::navOneBackback()
{
    navigateBack(history.oneBackward(current_history_index));
}

void BrowserTab::navOneForward()
{
    navigateBack(history.oneForward(current_history_index));
}

void BrowserTab::scrollToAnchor(QString const & anchor)
{
    qDebug() << "scroll to anchor" << anchor;
    this->ui->text_browser->scrollToAnchor(anchor);
}

void BrowserTab::reloadPage()
{
    if(current_location.isValid())
        this->navigateTo(this->current_location, DontPush);
}

void BrowserTab::on_url_bar_returnPressed()
{
    QUrl url { this->ui->url_bar->text() };

    if(url.scheme().isEmpty()) {
        url = QUrl { "gemini://" + this->ui->url_bar->text() };
    }

    this->navigateTo(url, PushImmediate);
}

void BrowserTab::on_refresh_button_clicked()
{
    reloadPage();
}

void BrowserTab::on_requestFailed(const QString &reason)
{
    this->setErrorMessage(QString("Request failed:\n%1").arg(reason));
}

QString size_human(int size)
{
    float num = size;
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext())
     {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

void BrowserTab::on_requestComplete(const QByteArray &data, const QString &mime)
{
    qDebug() << "Loaded" << data.length() << "bytes of type" << mime;

    this->graphics_scene.clear();
    this->ui->text_browser->setText("");

    ui->text_browser->setStyleSheet("");

    enum DocumentType { Text, Image, Media };

    DocumentType doc_type = Text;
    std::unique_ptr<QTextDocument> document;

    this->outline.clear();

    auto doc_style = mainWindow->current_style.derive(this->current_location);

    this->ui->text_browser->setStyleSheet(QString("QTextBrowser { background-color: %1; }").arg(doc_style.background_color.name()));

    if(mime.startsWith("text/gemini")) {
        document = GeminiRenderer::render(
            data,
            this->current_location,
            doc_style,
            this->outline);
    }
    else if(mime.startsWith("text/gophermap")) {
        document = GophermapRenderer::render(
            data,
            this->current_location,
            doc_style);
    }
    else if(mime.startsWith("text/html")) {
        document = std::make_unique<QTextDocument>();

        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());
        document->setHtml(QString::fromUtf8(data));
    }
#if defined(QT_FEATURE_textmarkdownreader)
    else if(mime.startsWith("text/markdown")) {
        document = std::make_unique<QTextDocument>();
        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());

        document->setMarkdown(QString::fromUtf8(data));
    }
#endif
    else if(mime.startsWith("text/")) {
        document = std::make_unique<QTextDocument>();
        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());
        document->setPlainText(QString::fromUtf8(data));
    }
    else if(mime.startsWith("image/")) {
        doc_type = Image;

        QImage img;
        if(img.loadFromData(data, nullptr))
        {
            this->graphics_scene.addPixmap(QPixmap::fromImage(img));
        }
        else
        {
            this->graphics_scene.addText("Failed to load picture!");
        }

        this->ui->graphics_browser->fitInView(graphics_scene.sceneRect(), Qt::KeepAspectRatio);
    }
    else if(mime.startsWith("video/") or mime.startsWith("audio/")) {
        doc_type = Media;
        this->ui->media_browser->setMedia(data, this->current_location, mime);
    }
    else {
        document = std::make_unique<QTextDocument>();
        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());

        document->setMarkdown(QString(R"md(You accessed an unsupported media type!

Use the *File* menu to save the file to your local disk or navigate somewhere else. I cannot display this for you. ☹

Info:
```
MIME Type: %1
File Size: %2
```
)md").arg(mime).arg(size_human(data.size())));
    }

    assert((document != nullptr) == (doc_type == Text));

    this->ui->text_browser->setVisible(doc_type == Text);
    this->ui->graphics_browser->setVisible(doc_type == Image);
    this->ui->media_browser->setVisible(doc_type == Media);

    this->ui->text_browser->setDocument(document.get());
    this->current_document = std::move(document);

    emit this->locationChanged(this->current_location);

    QString title = this->current_location.toString();
    emit this->titleChanged(title);

    this->successfully_loaded = true;

    if(this->push_to_history_after_load) {
        this->pushToHistory(this->current_location);
        this->push_to_history_after_load = false;
    }

    this->updateUI();
}

void BrowserTab::on_protocolViolation(const QString &reason)
{
    this->setErrorMessage(QString("Protocol violation:\n%1").arg(reason));
}

void BrowserTab::on_inputRequired(const QString &query)
{
    QInputDialog dialog { this };

    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setLabelText(query);

    if(dialog.exec() != QDialog::Accepted) {
        setErrorMessage(QString("Site requires input:\n%1").arg(query));
        return;
    }

    QUrl new_location = current_location;
    new_location.setQuery(dialog.textValue());
    this->navigateTo(new_location, DontPush);
}

void BrowserTab::on_redirected(const QUrl &uri, bool is_permanent)
{
    if(redirection_count >= 5) {
        setErrorMessage("Too many redirections!");
        return;
    }
    else {
        if(gemini_client.startRequest(uri)) {
            redirection_count += 1;
            this->current_location = uri;
            this->ui->url_bar->setText(uri.toString());
        }
    }
}

void BrowserTab::on_temporaryFailure(TemporaryFailure reason, const QString &info)
{
    switch(reason)
    {
    case TemporaryFailure::cgi_error:
        setErrorMessage(QString("CGI Error\n%1").arg(info));
        break;
    case TemporaryFailure::slow_down:
        setErrorMessage(QString("Slow Down\n%1").arg(info));
        break;
    case TemporaryFailure::proxy_error:
        setErrorMessage(QString("Proxy Error\n%1").arg(info));
        break;
    case TemporaryFailure::unspecified:
        setErrorMessage(QString("Temporary Failure\n%1").arg(info));
        break;
    case TemporaryFailure::server_unavailable:
        setErrorMessage(QString("Server Unavailable\n%1").arg(info));
        break;
    }
}

void BrowserTab::on_permanentFailure(PermanentFailure reason, const QString &info)
{
    switch(reason)
    {
    case PermanentFailure::gone:
        setErrorMessage(QString("Gone\n%1").arg(info));
        break;
    case PermanentFailure::not_found:
        setErrorMessage(QString("Not Found\n%1").arg(info));
        break;
    case PermanentFailure::bad_request:
        setErrorMessage(QString("Bad Request\n%1").arg(info));
        break;
    case PermanentFailure::unspecified:
        setErrorMessage(QString("Permanent Failure\n%1").arg(info));
        break;
    case PermanentFailure::proxy_request_required:
        setErrorMessage(QString("Proxy Request Required\n%1").arg(info));
        break;
    }
}

void BrowserTab::on_transientCertificateRequested(const QString &reason)
{
    QMessageBox::warning(this, "Kristall", "Transient certificate requirested:\n" + reason);
    this->updateUI();
}

void BrowserTab::on_authorisedCertificateRequested(const QString &reason)
{
    QMessageBox::warning(this, "Kristall", "Authorized certificate requirested:\n" + reason);
    this->updateUI();
}

void BrowserTab::on_certificateRejected(CertificateRejection reason, const QString &info)
{
    switch(reason)
    {
    case CertificateRejection::unspecified:
        setErrorMessage(QString("Certificate Rejected\n%1").arg(info));
        break;
    case CertificateRejection::not_accepted:
        setErrorMessage(QString("Certificate not accepted\n%1").arg(info));
        break;
    case CertificateRejection::future_certificate_rejected:
        setErrorMessage(QString("Certificate is not yet valid\n%1").arg(info));
        break;
    case CertificateRejection::expired_certificate_rejected:
        setErrorMessage(QString("Certificate expired\n%1").arg(info));
        break;
    }
}

void BrowserTab::on_linkHovered(const QString &url)
{
    this->mainWindow->setUrlPreview(QUrl(url));
}

void BrowserTab::setErrorMessage(const QString &msg)
{
    // this->page.setContent(QString("An error happened:\n%0").arg(msg).toUtf8(), "text/plain charset=utf-8");
    QMessageBox::warning(this, "Kristall", msg);
    this->updateUI();
}

void BrowserTab::pushToHistory(const QUrl &url)
{
    this->current_history_index = this->history.pushUrl(this->current_history_index, url);
    this->updateUI();
}

void BrowserTab::on_fav_button_clicked()
{
    if(this->ui->fav_button->isChecked()) {
        this->mainWindow->favourites.add(this->current_location);
    } else {
        this->mainWindow->favourites.remove(this->current_location);
    }

    this->updateUI();
}


void BrowserTab::on_text_browser_anchorClicked(const QUrl &url)
{
    qDebug() << url;

    QUrl real_url = url;
    if(real_url.isRelative())
        real_url = this->current_location.resolved(url);

    if(not mainWindow->protocols.isSchemeSupported(real_url.scheme())) {
        QMessageBox::warning(this, "Kristall", QString("Unsupported url: %1").arg(real_url.toString()));
    }
    else {
        this->navigateTo(real_url, PushAfterSuccess);
    }
}

void BrowserTab::on_text_browser_highlighted(const QUrl &url)
{
    QUrl real_url = url;
    if(real_url.isRelative())
        real_url = this->current_location.resolved(url);
    this->mainWindow->setUrlPreview(real_url);
}

void BrowserTab::on_stop_button_clicked()
{
    gemini_client.cancelRequest();
    web_client.cancelRequest();
    gopher_client.cancelRequest();
}

void BrowserTab::on_back_button_clicked()
{
    navOneBackback();
}

void BrowserTab::on_forward_button_clicked()
{
    navOneForward();
}

void BrowserTab::updateUI()
{
    this->ui->back_button->setEnabled(history.oneBackward(current_history_index).isValid());
    this->ui->forward_button->setEnabled(history.oneForward(current_history_index).isValid());

    this->ui->refresh_button->setVisible(this->successfully_loaded);
    this->ui->stop_button->setVisible(not this->successfully_loaded);

    this->ui->fav_button->setEnabled(this->successfully_loaded);
    this->ui->fav_button->setChecked(this->mainWindow->favourites.contains(this->current_location));
}
