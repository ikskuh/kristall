#include "browsertab.hpp"
#include "ui_browsertab.h"
#include "mainwindow.hpp"

#include <QTabWidget>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>

BrowserTab::BrowserTab(MainWindow * mainWindow) :
    QWidget(nullptr),
    ui(new Ui::BrowserTab),
    mainWindow(mainWindow),
    page(mainWindow),
    outline()
{
    ui->setupUi(this);

    connect(&gemini_client, &GeminiClient::requestComplete, this, &BrowserTab::on_gemini_complete);
    connect(&gemini_client, &GeminiClient::protocolViolation, this, &BrowserTab::on_protocolViolation);
    connect(&gemini_client, &GeminiClient::inputRequired, this, &BrowserTab::on_inputRequired);
    connect(&gemini_client, &GeminiClient::redirected, this, &BrowserTab::on_redirected);
    connect(&gemini_client, &GeminiClient::temporaryFailure, this, &BrowserTab::on_temporaryFailure);
    connect(&gemini_client, &GeminiClient::permanentFailure, this, &BrowserTab::on_permanentFailure);
    connect(&gemini_client, &GeminiClient::transientCertificateRequested, this, &BrowserTab::on_transientCertificateRequested);
    connect(&gemini_client, &GeminiClient::authorisedCertificateRequested, this, &BrowserTab::on_authorisedCertificateRequested);
    connect(&gemini_client, &GeminiClient::certificateRejected, this, &BrowserTab::on_certificateRejected);

    connect(&page, &QWebEnginePage::linkHovered, this, &BrowserTab::on_linkHovered);
    connect(&page, &GeminiWebPage::navigationRequest, this, &BrowserTab::on_navigationRequest);

    ui->content->setPage(&page);

    this->updateUI();
}

BrowserTab::~BrowserTab()
{
    delete ui;
}

void BrowserTab::navigateTo(const QUrl &url)
{
    if(url.scheme() != "gemini") {
        QMessageBox::warning(this, "Kristall", "Unsupported uri scheme: " + url.scheme());
        return;
    }
    this->current_location = url;
    this->ui->url_bar->setText(url.toString());

    if(not gemini_client.cancelRequest()) {
        QMessageBox::warning(this, "Kristall", "Unsupported uri scheme: " + url.scheme());
        return;
    }

    this->redirection_count = 0;
    this->successfully_loaded = false;

    gemini_client.startRequest(url);

    this->updateUI();
}

void BrowserTab::on_menu_button_clicked()
{
    QMenu menu;
    connect(menu.addAction("Add Tab"), &QAction::triggered, mainWindow, &MainWindow::addEmptyTab);
    connect(menu.addAction("Quit"), &QAction::triggered, &QApplication::quit);
    menu.exec(QCursor::pos());
}

void BrowserTab::on_url_bar_returnPressed()
{
    this->navigateTo(this->ui->url_bar->text());
}

void BrowserTab::on_content_titleChanged(const QString &title)
{
    this->setWindowTitle(title);
}

void BrowserTab::on_content_loadStarted()
{
    this->ui->refresh_button->setEnabled(false);
}

void BrowserTab::on_content_loadFinished(bool ok)
{
    this->ui->refresh_button->setEnabled(true);
}

void BrowserTab::on_content_urlChanged(const QUrl &url)
{
    // qDebug() << "url changed to" << url;
    // this->ui->url_bar->setText(url.toString());
}

void BrowserTab::on_refresh_button_clicked()
{
    if(current_location.isValid())
        this->navigateTo(this->current_location);
}

void BrowserTab::on_gemini_complete(const QByteArray &data, const QString &mime)
{
    if(mime.startsWith("text/gemini")) {
        this->page.setHtml(translateGeminiToHtml(data, this->outline), this->current_location);
    } else {
        this->page.setContent(data, mime);
    }
    this->successfully_loaded = true;
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
    this->navigateTo(new_location);
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

void BrowserTab::on_navigationRequest(const QUrl &url, bool &allow)
{
    if(url.scheme() != "gemini") {
        QMessageBox::warning(this, "Kristall", QString("Unsupported url: %1").arg(url.toString()));
    }
    else {
        this->navigateTo(url);
        allow = false;
    }
}

void BrowserTab::setErrorMessage(const QString &msg)
{
    // this->page.setContent(QString("An error happened:\n%0").arg(msg).toUtf8(), "text/plain charset=utf-8");
    QMessageBox::warning(this, "Kristall", msg);
    this->updateUI();
}

void BrowserTab::pushToHistory(const QUrl &url)
{
    this->navigation_history.append(url);
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


void BrowserTab::updateUI()
{
    this->ui->back_button->setEnabled(this->navigation_history.size() > 0);
    this->ui->forward_button->setEnabled(false);

    this->ui->refresh_button->setEnabled(this->successfully_loaded);

    this->ui->fav_button->setEnabled(this->successfully_loaded);
    this->ui->fav_button->setChecked(this->mainWindow->favourites.contains(this->current_location));
}

QByteArray BrowserTab::translateGeminiToHtml(const QByteArray &input, DocumentOutlineModel & outline)
{
    QByteArray result;
    result.append(QString(R"html(<!doctype html>
<html>
    <head>
        <meta charset="UTF-8">
    </head>
    <body>
)html").toUtf8());

    bool verbatim = false;
    bool listing = false;

    outline.beginBuild();

    QList<QByteArray> lines = input.split('\n');
    for(auto const & line : lines)
    {
        if(verbatim) {
            if(listing) {
                result.append("</ul>\n");
            }
            listing = false;

            if(line.startsWith("```")) {
                verbatim = false;
                result.append("</pre><br>\n");
            }
            else {
                result.append(line);
                result.append("\n");
            }
        } else {
            if(line.startsWith("*")) {
                if(not listing) {
                    result.append("<ul>\n");
                }
                listing = true;

                result.append("<li>");
                result.append(line.mid(1).trimmed());
                result.append("</li>");
                continue;
            } else {
                if(listing) {
                    result.append("</ul>\n");
                }
                listing = false;
            }

            if(line.startsWith("###")) {
                result.append("<h3>");
                outline.appendH3(line.mid(3).trimmed());
                result.append(line.mid(3).trimmed());
                result.append("</h3>");
            }
            else if(line.startsWith("##")) {
                result.append("<h2>");
                outline.appendH2(line.mid(2).trimmed());
                result.append(line.mid(2).trimmed());
                result.append("</h2>");
            }
            else if(line.startsWith("#")) {
                result.append("<h1>");
                outline.appendH1(line.mid(1).trimmed());
                result.append(line.mid(1).trimmed());
                result.append("</h1>");
            }
            else if(line.startsWith("=>")) {
                auto const part = line.mid(2).trimmed();

                QByteArray link, title;

                int index = -1;
                for(int i = 0; i < part.size(); i++) {
                    if(isspace(part[i])) {
                        index = i;
                        break;
                    }
                }

                if(index > 0) {
                    link = part.mid(0, index);
                    title = part.mid(index + 1);
                } else {
                    link = part;
                    title = part;
                }

                // qDebug() << link << title;

                result.append("<a href=\"");
                result.append(link);
                result.append("\">");
                result.append(title);
                result.append("</a><br>\n");
            }
            else if(line.startsWith("```")) {
                verbatim = true;
                result.append("<pre>");
            }
            else {
                result.append(line);
                result.append("<br>\n");
            }
        }
    }

    outline.endBuild();

    result.append(QString(R"html(
    </body>
</html>
)html").toUtf8());
    return result;
}
