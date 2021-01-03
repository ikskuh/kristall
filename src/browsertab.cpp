#include "browsertab.hpp"
#include "ui_browsertab.h"
#include "mainwindow.hpp"

#include "renderers/gophermaprenderer.hpp"
#include "renderers/geminirenderer.hpp"
#include "renderers/plaintextrenderer.hpp"
#include "renderers/markdownrenderer.hpp"

#include "mimeparser.hpp"

#include "dialogs/settingsdialog.hpp"
#include "dialogs/certificateselectiondialog.hpp"

#include "protocols/geminiclient.hpp"
#include "protocols/webclient.hpp"
#include "protocols/gopherclient.hpp"
#include "protocols/fingerclient.hpp"
#include "protocols/abouthandler.hpp"
#include "protocols/filehandler.hpp"

#include "ioutil.hpp"
#include "kristall.hpp"

#include <cassert>
#include <QTabWidget>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QImageReader>
#include <QClipboard>
#include <QDesktopServices>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QRegularExpression>
#include <iconv.h>

BrowserTab::BrowserTab(MainWindow *mainWindow) : QWidget(nullptr),
                                                 ui(new Ui::BrowserTab),
                                                 mainWindow(mainWindow),
                                                 current_handler(nullptr),
                                                 outline(),
                                                 graphics_scene()
{
    ui->setupUi(this);

    this->setUiDensity(kristall::options.ui_density);

    addProtocolHandler<GeminiClient>();
    addProtocolHandler<FingerClient>();
    addProtocolHandler<GopherClient>();
    addProtocolHandler<WebClient>();
    addProtocolHandler<AboutHandler>();
    addProtocolHandler<FileHandler>();

    this->updateUI();

    this->ui->search_bar->setVisible(false);

    this->ui->media_browser->setVisible(false);
    this->ui->graphics_browser->setVisible(false);
    this->ui->text_browser->setVisible(true);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    this->ui->text_browser->setTabStopDistance(40);
#else
    this->ui->text_browser->setTabStopWidth(40);
#endif

    this->ui->text_browser->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this->ui->url_bar, &SearchBar::escapePressed, this, &BrowserTab::on_url_bar_escapePressed);

    this->network_timeout_timer.setSingleShot(true);
    this->network_timeout_timer.setTimerType(Qt::PreciseTimer);

    connect(&this->network_timeout_timer, &QTimer::timeout, this, &BrowserTab::on_networkTimeout);



    {
        QShortcut * sc = new QShortcut(QKeySequence("Ctrl+F"), this);
        connect(sc, &QShortcut::activated, this, &BrowserTab::on_focusSearchbar);
    }
    {
        QShortcut * sc = new QShortcut(QKeySequence("F3"), this);
        connect(sc, &QShortcut::activated, this, &BrowserTab::on_search_next_clicked);
    }
    {
        QShortcut * sc = new QShortcut(QKeySequence("Shift+F3"), this);
        connect(sc, &QShortcut::activated, this, &BrowserTab::on_search_previous_clicked);
    }
    {
        QShortcut * sc = new QShortcut(QKeySequence("Escape"), this->ui->search_bar);
        connect(sc, &QShortcut::activated, this, &BrowserTab::on_close_search_clicked);
    }
}

BrowserTab::~BrowserTab()
{
    delete ui;
}

void BrowserTab::navigateTo(const QUrl &url, PushToHistory mode)
{
    if (kristall::protocols.isSchemeSupported(url.scheme()) != ProtocolSetup::Enabled)
    {
        QMessageBox::warning(this, "Kristall", "URI scheme not supported or disabled: " + url.scheme());
        return;
    }

    if ((this->current_handler != nullptr) and not this->current_handler->cancelRequest())
    {
        QMessageBox::warning(this, "Kristall", "Failed to cancel running request!");
        return;
    }

    this->redirection_count = 0;
    this->successfully_loaded = false;
    this->timer.start();

    if(not this->startRequest(url, ProtocolHandler::Default)) {
        QMessageBox::critical(this, "Kristall", QString("Failed to execute request to %1").arg(url.toString()));
        return;
    }

    if(mode == PushImmediate) {
        pushToHistory(url);
    }

    this->updateUI();
}

void BrowserTab::navigateBack(const QModelIndex &history_index)
{
    auto url = history.get(history_index);

    if (url.isValid())
    {
        current_history_index = history_index;
        navigateTo(url, DontPush);
    }
}

void BrowserTab::navOneBackward()
{
    navigateBack(history.oneBackward(current_history_index));
}

void BrowserTab::navOneForward()
{
    navigateBack(history.oneForward(current_history_index));
}

void BrowserTab::scrollToAnchor(QString const &anchor)
{
    qDebug() << "scroll to anchor" << anchor;
    this->ui->text_browser->scrollToAnchor(anchor);
}

void BrowserTab::reloadPage()
{
    if (current_location.isValid())
        this->navigateTo(this->current_location, DontPush);
}

void BrowserTab::toggleIsFavourite()
{
    toggleIsFavourite(not this->ui->fav_button->isChecked());
}

void BrowserTab::toggleIsFavourite(bool shouldBeFavourite)
{
    // isFavourite is the "new" state of the checkbox, so when it's true
    // we yet need to add it.
    if (shouldBeFavourite)
    {
        kristall::favourites.addUnsorted(this->current_location, this->page_title);
    }
    else
    {
        auto answer = QMessageBox::question(
            this,
            "Kristall",
            tr("Do you really want to remove this page from your favourites?")
        );
        if(answer != QMessageBox::Yes)
        {
            this->updateUI();
            return;
        }
        kristall::favourites.removeUrl(this->current_location);
    }

    this->updateUI();
}

void BrowserTab::focusUrlBar()
{
    this->ui->url_bar->setFocus(Qt::ShortcutFocusReason);
    this->ui->url_bar->selectAll();
}

void BrowserTab::focusSearchBar()
{
    if(not this->ui->search_bar->isVisible()) {
        this->ui->search_box->setText("");
    }
    this->ui->search_bar->setVisible(true);
    this->ui->search_box->setFocus();
    this->ui->search_box->selectAll();
}

void BrowserTab::openSourceView()
{
    QFont monospace_font("monospace");
    monospace_font.setStyleHint(QFont::Monospace);

    auto dialog = std::make_unique<QDialog>(this);
    dialog->setWindowTitle(QString("Source of %0").arg(this->current_location.toString()));

    auto layout = new QVBoxLayout(dialog.get());
    dialog->setLayout(layout);

    auto hint = new QLabel(dialog.get());
    hint->setText(QString("Mime type: %0").arg(current_mime.toString()));
    layout->addWidget(hint);

    auto text = new QPlainTextEdit(dialog.get());
    text->setPlainText(QString::fromUtf8(current_buffer));
    text->setReadOnly(true);
    text->setFont(monospace_font);
    text->setWordWrapMode(QTextOption::NoWrap);
    layout->addWidget(text);

    auto buttons = new QDialogButtonBox(dialog.get());
    buttons->setStandardButtons(QDialogButtonBox::Ok);
    layout->addWidget(buttons);

    connect(buttons->button(QDialogButtonBox::Ok), &QPushButton::pressed, dialog.get(), &QDialog::accept);

    dialog->resize(640, 480);
    dialog->exec();
}

void BrowserTab::on_url_bar_returnPressed()
{
    QString urltext = this->ui->url_bar->text().trimmed();

    // Expand '~' to user's home directory.
    static const QString PREFIX_HOME = "file://~";
    if (urltext.startsWith(PREFIX_HOME))
        urltext = "file://" + QDir::homePath() + urltext.remove(0, PREFIX_HOME.length());

    QUrl url { urltext };

    if (url.scheme().isEmpty())
    {
        url = QUrl{"gemini://" + this->ui->url_bar->text()};
    }

    this->ui->url_bar->clearFocus();

    this->navigateTo(url, PushImmediate);
}

void BrowserTab::on_url_bar_escapePressed()
{
    this->setUrlBarText(this->current_location.toString(QUrl::FullyEncoded));
}

void BrowserTab::on_url_bar_focused()
{
    this->updateUrlBarStyle();
}

void BrowserTab::on_url_bar_blurred()
{
    this->updateUrlBarStyle();
}

void BrowserTab::on_refresh_button_clicked()
{
    reloadPage();
}

void BrowserTab::on_networkError(ProtocolHandler::NetworkError error_code, const QString &reason)
{
    this->network_timeout_timer.stop();

    QString file_name;
    switch(error_code)
    {
    case ProtocolHandler::UnknownError: file_name = "UnknownError.gemini"; break;
    case ProtocolHandler::ProtocolViolation: file_name = "ProtocolViolation.gemini"; break;
    case ProtocolHandler::HostNotFound: file_name = "HostNotFound.gemini"; break;
    case ProtocolHandler::ConnectionRefused: file_name = "ConnectionRefused.gemini"; break;
    case ProtocolHandler::ResourceNotFound: file_name = "ResourceNotFound.gemini"; break;
    case ProtocolHandler::BadRequest: file_name = "BadRequest.gemini"; break;
    case ProtocolHandler::ProxyRequest: file_name = "ProxyRequest.gemini"; break;
    case ProtocolHandler::InternalServerError: file_name = "InternalServerError.gemini"; break;
    case ProtocolHandler::InvalidClientCertificate: file_name = "InvalidClientCertificate.gemini"; break;
    case ProtocolHandler::UntrustedHost: file_name = "UntrustedHost.gemini"; break;
    case ProtocolHandler::MistrustedHost: file_name = "MistrustedHost.gemini"; break;
    case ProtocolHandler::Unauthorized: file_name = "Unauthorized.gemini"; break;
    case ProtocolHandler::TlsFailure: file_name = "TlsFailure.gemini"; break;
    case ProtocolHandler::Timeout: file_name = "Timeout.gemini"; break;
    }
    file_name = ":/error_page/" + file_name;

    QFile file_src { file_name };

    if(not file_src.open(QFile::ReadOnly)) {
        assert(false);
    }

    auto contents = QString::fromUtf8(file_src.readAll()).arg(reason).toUtf8();

    this->is_internal_location = true;

    this->on_requestComplete(
        contents,
        "text/gemini");

    this->updateUI();
}

void BrowserTab::on_networkTimeout()
{
    if(this->current_handler != nullptr) {
        this->current_handler->cancelRequest();
    }
    on_networkError(ProtocolHandler::Timeout, "The server didn't respond in time.");
}

void BrowserTab::on_focusSearchbar()
{
    this->focusSearchBar();
}

void BrowserTab::on_certificateRequired(const QString &reason)
{
    this->network_timeout_timer.stop();

    if (not trySetClientCertificate(reason))
    {
        setErrorMessage(QString("The page requested a authorized client certificate, but none was provided.\r\nOriginal query was: %1").arg(reason));
    }
    else
    {
        this->navigateTo(this->current_location, DontPush);
    }
    this->updateUI();
}

void BrowserTab::on_hostCertificateLoaded(const QSslCertificate &cert)
{
    this->current_server_certificate = cert;
}

static QByteArray convertToUtf8(QByteArray const & input, QString const & charSet)
{
    auto charset_u8 = charSet.toUpper().toUtf8();

    // TRANSLIT will try to mix-match other code points to reflect to correct encoding
    iconv_t cd = iconv_open("UTF-8", charset_u8.data());
    if(cd == (iconv_t)-1) {
        return QByteArray { };
    }

    QByteArray result;

    char temp_buffer[4096];

#if defined(__NetBSD__)
    char const * input_ptr = reinterpret_cast<char const *>(input.data());
#else
    char * input_ptr = const_cast<char *>(reinterpret_cast<char const *>(input.data()));
#endif
    size_t input_size = input.size();

    while(input_size > 0)
    {
        char * out_ptr = temp_buffer;
        size_t out_size = sizeof(temp_buffer);

        size_t n = iconv(cd, &input_ptr, &input_size, &out_ptr, &out_size);
        if (n == size_t(-1))
        {
            if(errno == E2BIG) {
                // silently ignore E2BIG, as we will continue conversion in the next loop
            }
            else if(errno == EILSEQ) {
                // this is an invalid multibyte sequence.
                // append an "replacement character" and skip a byte
                if(input_size > 0) {
                    input_size --;
                    input_ptr++;
                    result.append(u8"�");
                }
            }
            else if(errno == EINVAL) {
                // the file ends with an invalid multibyte sequence.
                // just drop it and display the replacement-character
                if(input_size > 0) {
                    input_size --;
                    input_ptr++;
                    result.append(u8"�");
                }
            }
            else {
                perror("iconv conversion error");
                break;
            }
        }

        size_t len = out_ptr - temp_buffer;
        result.append(temp_buffer, len);
    }

    iconv_close(cd);

    return result;
}

void BrowserTab::on_requestComplete(const QByteArray &ref_data, const QString &mime_text)
{
    this->ui->media_browser->stopPlaying();
    this->network_timeout_timer.stop();

    QByteArray data = ref_data;
    MimeType mime = MimeParser::parse(mime_text);

    qDebug() << "Loaded" << ref_data.length() << "bytes of type" << mime.type << "/" << mime.subtype;
//    for(auto & key : mime.parameters.keys()) {
//        qDebug() << key << mime.parameters[key];
//    }

    auto charset = mime.parameter("charset", "utf-8").toUpper();
    if(not ref_data.isEmpty() and (mime.type == "text") and (charset != "UTF-8"))
    {
        auto temp = convertToUtf8(ref_data, charset);
        bool ok = (temp.size() > 0);
        if(ok) {
            data = std::move(temp);
        } else {
            auto response = QMessageBox::question(
                this,
                "Kristall",
                QString("Failed to convert input charset %1 to UTF-8. Cannot display the file.\r\nDo you want to display unconverted data anyways?").arg(charset)
            );

            if(response != QMessageBox::Yes) {
                setErrorMessage(QString("Failed to convert input charset %1 to UTF-8.").arg(charset));
                return;
            }
        }
    }

    this->successfully_loaded = true;
    this->page_title = "";

    renderPage(data, mime);

    this->updatePageTitle();

    this->updateUrlBarStyle();

    this->current_stats.file_size = ref_data.size();
    this->current_stats.mime_type = mime;
    this->current_stats.loading_time = this->timer.elapsed();
    emit this->fileLoaded(this->current_stats);

    this->updateMouseCursor(false);
}

void BrowserTab::renderPage(const QByteArray &data, const MimeType &mime)
{
    this->current_mime = mime;
    this->current_buffer = data;

    this->graphics_scene.clear();
    this->ui->text_browser->setText("");

    ui->text_browser->setStyleSheet("");

    enum DocumentType
    {
        Text,
        Image,
        Media
    };

    DocumentType doc_type = Text;
    std::unique_ptr<QTextDocument> document;

    this->outline.clear();

    auto doc_style = kristall::document_style.derive(this->current_location);

    this->ui->text_browser->setStyleSheet(QString("QTextBrowser { background-color: %1; color: %2; }").arg(doc_style.background_color.name(), doc_style.standard_color.name()));

    bool plaintext_only = (kristall::options.text_display == GenericSettings::PlainText);

    if (not plaintext_only and mime.is("text", "gemini"))
    {
        document = GeminiRenderer::render(
            data,
            this->current_location,
            doc_style,
            this->outline,
            &this->page_title);
    }
    else if (not plaintext_only and mime.is("text","gophermap"))
    {
        document = GophermapRenderer::render(
            data,
            this->current_location,
            doc_style);
    }
    else if (not plaintext_only and mime.is("text","html"))
    {
        document = std::make_unique<QTextDocument>();

        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());
        document->setDocumentMargin(doc_style.margin);

        // Strip inline styles from page, so they don't
        // conflict with user styles.
        QString page_html = QString::fromUtf8(data);
        page_html.replace(QRegularExpression("<style.*?>[\\S\\s]*?</style.*?>", QRegularExpression::CaseInsensitiveOption), "");

        // Strip bgcolor attribute from body. These can screw up user styles too.
        page_html.replace(QRegularExpression("<body.*bgcolor.*>", QRegularExpression::CaseInsensitiveOption), "<body>");

        document->setHtml(page_html);

        // Find page title in HTML
        // Split so we only look in the <head>
        QStringList head = page_html.split("</head>", QString::KeepEmptyParts, Qt::CaseInsensitive);
        if (head[0] != page_html)
        {
            // Split at first title tag.
            QStringList a = head[0].split("<title>", QString::KeepEmptyParts, Qt::CaseInsensitive);
            if (a[0] != head[0])
            {
                // Split at second tag.
                QStringList b = a[1].split("</title>", QString::KeepEmptyParts, Qt::CaseInsensitive);
                if (b[0] != a[1])
                {
                    QTextDocument title;
                    title.setHtml(b[0]);
                    this->page_title = title.toPlainText();
                }
            }
        }
    }
    else if (not plaintext_only and mime.is("text","x-kristall-theme"))
    {
        // ugly workaround for QSettings needing a file
        QFile temp_file { kristall::dirs::cache_root.absoluteFilePath("preview-theme.kthm") };

        if(temp_file.open(QFile::WriteOnly)) {
            IoUtil::writeAll(temp_file, data);
            temp_file.close();
        }

        QSettings temp_settings {
            temp_file.fileName(),
            QSettings::IniFormat
        };

        DocumentStyle preview_style;
        preview_style.load(temp_settings);

        QFile src { ":/about/style-preview.gemini" };
        src.open(QFile::ReadOnly);

        document = GeminiRenderer::render(
            src.readAll(),
            this->current_location,
            preview_style,
            this->outline);

        this->ui->text_browser->setStyleSheet(QString("QTextBrowser { background-color: %1; color: %2; }")
            .arg(preview_style.background_color.name(), preview_style.standard_color.name()));
    }
    else if (not plaintext_only and mime.is("text","markdown"))
    {
        document = MarkdownRenderer::render(
            data,
            this->current_location,
            doc_style,
            this->outline,
            this->page_title);
    }
    else if (mime.is("text"))
    {
        document = PlainTextRenderer::render(data, doc_style);
    }
    else if (mime.is("image"))
    {
        doc_type = Image;

        QBuffer buffer;
        buffer.setData(data);

        QImageReader reader{&buffer};
        reader.setAutoTransform(true);
        reader.setAutoDetectImageFormat(true);

        QImage img;
        if (reader.read(&img))
        {
            auto pixmap = QPixmap::fromImage(img);
            this->graphics_scene.addPixmap(pixmap);
            this->graphics_scene.setSceneRect(pixmap.rect());
        }
        else
        {
            this->graphics_scene.addText(QString("Failed to load picture:\r\n%1").arg(reader.errorString()));
        }

        this->ui->graphics_browser->setScene(&graphics_scene);

        auto *invoker = new QObject();
        connect(invoker, &QObject::destroyed, [this]() {
            this->ui->graphics_browser->fitInView(graphics_scene.sceneRect(), Qt::KeepAspectRatio);
        });
        invoker->deleteLater();

        this->ui->graphics_browser->fitInView(graphics_scene.sceneRect(), Qt::KeepAspectRatio);
    }
    else if (mime.is("video") or mime.is("audio"))
    {
        doc_type = Media;
        this->ui->media_browser->setMedia(data, this->current_location, mime.type);
    }
    else if (plaintext_only)
    {
        document = std::make_unique<QTextDocument>();
        document->setDefaultFont(doc_style.standard_font);
        document->setDefaultStyleSheet(doc_style.toStyleSheet());

        QString plain_data = QString(
            "Unsupported Media Type!\n"
            "\n"
            "Kristall cannot display the requested document\n"
            "To view this media, use the File menu to save it to your local drive, then open the saved file in another program that can display the document for you.\n\n"
            "Details:\n"
            "- MIME type: %1/%2\n"
            "- Size: %3\n"
        ).arg(mime.type, mime.subtype, IoUtil::size_human(data.size()));

        document->setPlainText(plain_data);
    }
    else
    {
        QString page_data = QString(
            "# Unsupported Media Type!\n"
            "\n"
            "Kristall cannot display the requested document.\n"
            "\n"
            "> To view this media, use the File menu to save it to your local drive, then open the saved file in another program that can display the document for you.\n"
            "\n"
            "```\n"
            "Details:\n"
            "- MIME type: %1/%2\n"
            "- Size: %3\n"
            "```\n"
        ).arg(mime.type, mime.subtype, IoUtil::size_human(data.size()));

        document = GeminiRenderer::render(
            page_data.toUtf8(),
            this->current_location,
            doc_style,
            this->outline,
            &this->page_title);
    }

    assert((document != nullptr) == (doc_type == Text));

    this->ui->text_browser->setVisible(doc_type == Text);
    this->ui->graphics_browser->setVisible(doc_type == Image);
    this->ui->media_browser->setVisible(doc_type == Media);

    this->ui->text_browser->setDocument(document.get());
    this->current_document = std::move(document);

    this->needs_rerender = false;

    emit this->locationChanged(this->current_location);

    this->updateUI();

    this->updateUrlBarStyle();
}

void BrowserTab::rerenderPage()
{
    this->renderPage(this->current_buffer, this->current_mime);
}

void BrowserTab::updatePageTitle()
{
    if (page_title.isEmpty())
    {
        page_title = this->current_location.toString();
    }

    // TODO: Shorten lengthy titles?

    emit this->titleChanged(this->page_title);
}


void BrowserTab::on_inputRequired(const QString &query, const bool is_sensitive)
{
    this->network_timeout_timer.stop();

    QInputDialog dialog{this};

    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setLabelText(query);
    if (is_sensitive) dialog.setTextEchoMode(QLineEdit::Password);

    while(true)
    {
        if (dialog.exec() != QDialog::Accepted)
        {
            setErrorMessage(QString("Site requires input:\n%1").arg(query));
            return;
        }

        QUrl new_location = current_location;
        new_location.setQuery(dialog.textValue());

        int len = new_location.toString(QUrl::FullyEncoded).toUtf8().size();
        if(len >= 1020) {
            QMessageBox::warning(
                this,
                "Kristall",
                tr("Your input message is too long. Your input is %1 bytes, but a maximum of %2 bytes are allowed.\r\nPlease cancel or shorten your input.").arg(len).arg(1020)
            );
        } else {
            this->navigateTo(new_location, DontPush);
            break;
        }
    }
}

void BrowserTab::on_redirected(QUrl uri, bool is_permanent)
{
    Q_UNUSED(is_permanent);

    this->network_timeout_timer.stop();

    // #79: Handle non-full url redirects
    if (uri.isRelative())
    {
        uri.setScheme(current_location.scheme());
        uri.setHost(current_location.host());
    }

    if (redirection_count >= kristall::options.max_redirections)
    {
        setErrorMessage(QString("Too many consecutive redirections. The last redirection would have redirected you to:\r\n%1").arg(uri.toString(QUrl::FullyEncoded)));
        return;
    }
    else
    {
        bool is_cross_protocol = (this->current_location.scheme() != uri.scheme());
        bool is_cross_host = (this->current_location.host() != uri.host());

        QString question;
        if(kristall::options.redirection_policy == GenericSettings::WarnAlways)
        {
            question = QString(
                "The location you visited wants to redirect you to another location:\r\n"
                "%1\r\n"
                "Do you want to allow the redirection?"
            ).arg(uri.toString(QUrl::FullyEncoded));
        }
        else if((kristall::options.redirection_policy & (GenericSettings::WarnOnHostChange | GenericSettings::WarnOnSchemeChange)) and is_cross_protocol and is_cross_host)
        {
            question = QString(
                "The location you visited wants to redirect you to another host and switch the protocol.\r\n"
                "Protocol: %1\r\n"
                "New Host: %2\r\n"
                "Do you want to allow the redirection?"
            ).arg(uri.scheme()).arg(uri.host());
        }
        else if((kristall::options.redirection_policy & GenericSettings::WarnOnSchemeChange) and is_cross_protocol)
        {
            question = QString(
                "The location you visited wants to switch the protocol.\r\n"
                "Protocol: %1\r\n"
                "Do you want to allow the redirection?"
            ).arg(uri.scheme());
        }
        else if((kristall::options.redirection_policy & GenericSettings::WarnOnHostChange) and is_cross_host)
        {
            question = QString(
                "The location you visited wants to redirect you to another host.\r\n"
                "New Host: %1\r\n"
                "Do you want to allow the redirection?"
            ).arg(uri.host());
        }

        if(is_cross_protocol or is_cross_host)
        {
            auto answer = QMessageBox::question(
                this,
                "Kristall",
                question
            );
            if(answer != QMessageBox::Yes) {
                setErrorMessage(QString("Redirection to %1 cancelled by user").arg(uri.toString()));
                return;
            }
        }

        if (this->startRequest(uri, ProtocolHandler::Default))
        {
            redirection_count += 1;
            this->current_location = uri;
            this->setUrlBarText(uri.toString(QUrl::FullyEncoded));
        }
        else
        {
            setErrorMessage(QString("Redirection to %1 failed").arg(uri.toString()));
        }
    }
}

void BrowserTab::setErrorMessage(const QString &msg)
{
    this->on_requestComplete(
        QString("An error happened:\r\n%0").arg(msg).toUtf8(),
        "text/plain charset=utf-8");

    this->updateUI();
}

void BrowserTab::pushToHistory(const QUrl &url)
{
    this->current_history_index = this->history.pushUrl(this->current_history_index, url);
    this->updateUI();
}

void BrowserTab::on_fav_button_clicked()
{
    toggleIsFavourite(this->ui->fav_button->isChecked());
}

void BrowserTab::on_text_browser_anchorClicked(const QUrl &url, bool open_in_new_tab)
{
    // Ctrl scheme is *always* the current tab, it's
    // used for fake-buttons
    if(url.scheme() == "kristall+ctrl")
    {
        bool is_theme_preview = this->current_mime.is("text", "x-kristall-theme");

        if(this->is_internal_location or is_theme_preview) {
            QString opt = url.path();
            qDebug() << "kristall control action" << opt;

            // this will bypass the TLS security
            if(not is_theme_preview and opt == "ignore-tls") {
                auto response = QMessageBox::question(
                    this,
                    "Kristall",
                    tr("This sites certificate could not be verified! This may be a man-in-the-middle attack on the server to send you malicious content (or the server admin made a configuration mistake).\r\nAre you sure you want to continue?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                );
                if(response == QMessageBox::Yes) {
                    this->startRequest(this->current_location, ProtocolHandler::IgnoreTlsErrors);
                }
            }
            //
            else if(not is_theme_preview and opt == "ignore-tls-safe") {
                this->startRequest(this->current_location, ProtocolHandler::IgnoreTlsErrors);
            }
            // Add this page to the list of trusted hosts and continue
            else if(not is_theme_preview and opt == "add-fingerprint") {
                auto answer = QMessageBox::question(
                    this,
                    "Kristall",
                    tr("Do you really want to add the server certificate to your list of trusted hosts?\r\nHost: %1")
                        .arg(this->current_location.host()),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes // that's a sane option here
                );
                if(answer != QMessageBox::Yes) {
                    return;
                }

                if(this->current_location.scheme() == "gemini") {
                    kristall::trust::gemini.addTrust(this->current_location, this->current_server_certificate);
                }
                else if(this->current_location.scheme() == "https") {
                    kristall::trust::https.addTrust(this->current_location, this->current_server_certificate);
                }
                else {
                    assert(false and "missing protocol implementation!");
                }

                this->startRequest(this->current_location, ProtocolHandler::Default);
            }
            else if(opt == "install-theme") {

                if(is_theme_preview)
                {
                    // ugly workaround for QSettings needing a file
                    QFile temp_file { kristall::dirs::cache_root.absoluteFilePath("preview-theme.kthm") };

                    if(temp_file.open(QFile::WriteOnly)) {
                        IoUtil::writeAll(temp_file, this->current_buffer);
                        temp_file.close();
                    }

                    QSettings temp_settings {
                        temp_file.fileName(),
                        QSettings::IniFormat
                    };

                    QString name;
                    if(auto name_var = temp_settings.value("name"); name_var.isNull())
                    {
                        QInputDialog input { this };
                        input.setInputMode(QInputDialog::TextInput);
                        input.setLabelText(tr("This style has no embedded name. Please enter a name for the preset:"));
                        input.setTextValue(this->current_location.fileName().split(".", QString::SkipEmptyParts).first());

                        if(input.exec() != QDialog::Accepted)
                            return;

                        name = input.textValue().trimmed();
                    }
                    else
                    {
                        name = name_var.toString();
                    }

                    auto answer = QMessageBox::question(
                        this,
                        "Kristall",
                        tr("Do you want to add the style %1 to your collection?").arg(name)
                    );
                    if(answer != QMessageBox::Yes)
                        return;

                    QString fileName;

                    int index = 0;
                    do
                    {
                        fileName = DocumentStyle::createFileNameFromName(name, index);
                        index += 1;
                    } while(kristall::dirs::styles.exists(fileName));

                    QFile target_file { kristall::dirs::styles.absoluteFilePath(fileName) };

                    if(target_file.open(QFile::WriteOnly)) {
                        IoUtil::writeAll(target_file, this->current_buffer);
                        target_file.close();
                    }

                    QSettings final_settings {
                        target_file.fileName(),
                        QSettings::IniFormat
                    };
                    final_settings.setValue("name", name);
                    final_settings.sync();

                    QMessageBox::information(
                        this,
                        "Kristall",
                        tr("The theme %1 was successfully added to your theme collection!").arg(name)
                    );
                }
                else
                {
                    qDebug() << "install-theme triggered from non-theme document!";
                }
            }
        } else {
            QMessageBox::critical(
                this,
                "Kristall",
                tr("Malicious site detected! This site tries to use the Kristall control scheme!\r\nA trustworthy site does not do this!").arg(this->current_location.host())
            );
        }
        return;
    }

    QUrl real_url = url;
    if (real_url.isRelative())
        real_url = this->current_location.resolved(url);

    auto support = kristall::protocols.isSchemeSupported(real_url.scheme());

    if (support == ProtocolSetup::Enabled)
    {
        if(open_in_new_tab) {
            mainWindow->addNewTab(false, real_url);
        } else {
            this->navigateTo(real_url, PushImmediate);
        }
    }
    else
    {
        if (kristall::options.use_os_scheme_handler)
        {
            if (not QDesktopServices::openUrl(url))
            {
                QMessageBox::warning(this, "Kristall", QString("Failed to start system URL handler for\r\n%1").arg(real_url.toString()));
            }
        }
        else if (support == ProtocolSetup::Disabled)
        {
            QMessageBox::warning(this, "Kristall", QString("The requested url uses a scheme that has been disabled in the settings:\r\n%1").arg(real_url.toString()));
        }
        else
        {
            QMessageBox::warning(this, "Kristall", QString("The requested url cannot be processed by Kristall:\r\n%1").arg(real_url.toString()));
        }
    }
}

void BrowserTab::on_text_browser_highlighted(const QUrl &url)
{
    if (url.isValid() and not (url.scheme() == "kristall+ctrl"))
    {
        QUrl real_url = url;
        if (real_url.isRelative())
            real_url = this->current_location.resolved(url);
        this->mainWindow->setUrlPreview(real_url);
    }
    else
    {
        this->mainWindow->setUrlPreview(QUrl{});
    }
}

void BrowserTab::on_stop_button_clicked()
{
    if(this->current_handler != nullptr) {
        this->current_handler->cancelRequest();
    }
    this->updateUI();
}

void BrowserTab::on_home_button_clicked()
{
    this->navigateTo(QUrl(kristall::options.start_page), BrowserTab::PushImmediate);
}

void BrowserTab::on_requestProgress(qint64 transferred)
{
    this->current_stats.file_size = transferred;
    this->current_stats.mime_type = MimeType { };
    this->current_stats.loading_time = this->timer.elapsed();
    emit this->fileLoaded(this->current_stats);

    this->network_timeout_timer.stop();
    this->network_timeout_timer.start(kristall::options.network_timeout);
}

void BrowserTab::on_back_button_clicked()
{
    navOneBackward();
}

void BrowserTab::on_forward_button_clicked()
{
    navOneForward();
}

void BrowserTab::updateUI()
{
    this->ui->back_button->setEnabled(history.oneBackward(current_history_index).isValid());
    this->ui->forward_button->setEnabled(history.oneForward(current_history_index).isValid());

    this->ui->home_button->setVisible(kristall::options.enable_home_btn);

    bool in_progress = (this->current_handler != nullptr) and this->current_handler->isInProgress();

    this->ui->refresh_button->setVisible(not in_progress);
    this->ui->stop_button->setVisible(in_progress);

    this->ui->fav_button->setEnabled(this->successfully_loaded);
    this->ui->fav_button->setChecked(kristall::favourites.containsUrl(this->current_location));
}

void BrowserTab::setUrlBarText(const QString & text)
{
    this->ui->url_bar->setText(text);
    this->updateUrlBarStyle();
}

void BrowserTab::updateUrlBarStyle()
{
    // https://stackoverflow.com/a/14424003
    const auto setLineEditTextFormat =
        [](QLineEdit* l, const QList<QTextLayout::FormatRange>& f)
    {
        if (!l) return;

        QList<QInputMethodEvent::Attribute> attr;
        foreach (const QTextLayout::FormatRange& fr, f)
        {
            attr.append(QInputMethodEvent::Attribute(
                QInputMethodEvent::TextFormat,
                fr.start - l->cursorPosition(),
                fr.length,
                fr.format));
        }
        QInputMethodEvent event(QString(), attr);
        QCoreApplication::sendEvent(l, &event);
    };

    QUrl url { this->ui->url_bar->text().trimmed() };

    // Set all text to default colour if url bar
    // is focused, is at an internal location (like about:...),
    // or has an invalid URL.
    if (!kristall::options.fancy_urlbar ||
        this->ui->url_bar->hasFocus() ||
        !url.isValid() ||
        this->is_internal_location)
    {
        // Disable styling
        if (!this->no_url_style)
        {
            setLineEditTextFormat(this->ui->url_bar,
                QList<QTextLayout::FormatRange>());
            this->no_url_style = true;
        }
        return;
    }

    this->no_url_style = false;

    // Styling enabled: 'authority' (hostname, port, etc) of
    // the URL is highlighted (i.e default colour),
    // the rest is in grey-ish colour
    //
    // Example:
    //
    // gemini://an.example.com:1965/index.gmi
    // ^-------^
    //   grey   ^-----------------^
    //               default
    //                             ^--------^
    //                                grey

    QList<QTextLayout::FormatRange> formats;

    // We only need to create one style, which is the
    // non-authority colour text (grey-ish, determined by theme in kristall:setTheme)
    // The rest of the text is in default theme foreground colour.
    QTextCharFormat f;
    f.setForeground(kristall::options.fancy_urlbar_dim_colour);

    // Create format range for left-side of URL
    QTextLayout::FormatRange fr_left;
    fr_left.start = 0;
    fr_left.length = url.scheme().length() + strlen("://");
    fr_left.format = f;
    formats.append(fr_left);

    // Create format range for right-side of URL (if we have one)
    if (url.scheme() != "file" && !url.path().isEmpty())
    {
        QTextLayout::FormatRange fr_right;

        fr_right.start = fr_left.length + url.authority().length();
        fr_right.length = url.toString(QUrl::FullyEncoded).length() - fr_right.start;
        fr_right.format = f;
        formats.append(fr_right);
    }

    // Finally, apply the colour formatting.
    setLineEditTextFormat(this->ui->url_bar, formats);
}

void BrowserTab::setUiDensity(UIDensity density)
{
    switch (density)
    {
    case UIDensity::compact:
    {
        this->ui->layout_main->setContentsMargins(0, 0, 0, 0);
        this->ui->layout_toolbar->setContentsMargins(8, 0, 8, 0);
    } break;

    case UIDensity::classic:
    {
        this->ui->layout_main->setContentsMargins(0, 9, 0, 9);
        this->ui->layout_toolbar->setContentsMargins(18, 9, 18, 9);
    } break;
    }
}

bool BrowserTab::trySetClientCertificate(const QString &query)
{
    CertificateSelectionDialog dialog{this};

    dialog.setServerQuery(query);

    if (dialog.exec() != QDialog::Accepted)
    {
        this->disableClientCertificate();
        return false;
    }

    return this->enableClientCertificate(dialog.identity());
}

void BrowserTab::resetClientCertificate()
{
    if (this->current_identity.isValid() and not this->current_identity.is_persistent)
    {
        auto respo = QMessageBox::question(this, "Kristall", "You currently have a transient session active!\r\nIf you disable the session, you will not be able to restore it. Continue?");
        if (respo != QMessageBox::Yes)
        {
            this->ui->enable_client_cert_button->setChecked(true);
            return;
        }
    }

    this->disableClientCertificate();
}

void BrowserTab::addProtocolHandler(std::unique_ptr<ProtocolHandler> &&handler)
{
    connect(handler.get(), &ProtocolHandler::requestProgress, this, &BrowserTab::on_requestProgress);
    connect(handler.get(), &ProtocolHandler::requestComplete, this, &BrowserTab::on_requestComplete);
    connect(handler.get(), &ProtocolHandler::redirected, this, &BrowserTab::on_redirected);
    connect(handler.get(), &ProtocolHandler::inputRequired, this, &BrowserTab::on_inputRequired);
    connect(handler.get(), &ProtocolHandler::networkError, this, &BrowserTab::on_networkError);
    connect(handler.get(), &ProtocolHandler::certificateRequired, this, &BrowserTab::on_certificateRequired);
    connect(handler.get(), &ProtocolHandler::hostCertificateLoaded, this, &BrowserTab::on_hostCertificateLoaded);

    this->protocol_handlers.emplace_back(std::move(handler));
}

bool BrowserTab::startRequest(const QUrl &url, ProtocolHandler::RequestOptions options)
{
    this->updateMouseCursor(true);

    this->current_server_certificate = QSslCertificate { };

    this->current_handler = nullptr;
    for(auto & ptr : this->protocol_handlers)
    {
        if(ptr->supportsScheme(url.scheme())) {
            this->current_handler = ptr.get();
            break;
        }
    }

    assert((this->current_handler != nullptr) and "If this error happens, someone forgot to add a new protocol handler class in the constructor. Shame on the programmer!");

    auto const try_enable_certificate = [&]() -> bool {
        if(this->current_identity.isValid()) {
            if(not this->current_handler->enableClientCertificate(this->current_identity)) {
                auto answer = QMessageBox::question(
                    this,
                    "Kristall",
                    tr("You requested a %1-URL with a client certificate, but these are not supported for this scheme. Continue?").arg(url.scheme())
                );
                if(answer != QMessageBox::Yes)
                    return false;
                this->disableClientCertificate();
            }
        } else {
            this->disableClientCertificate();
        }
        return true;
    };
    if(not try_enable_certificate())
        return false;

    if(this->current_identity.isValid() and (url.host() != this->current_location.host())) {
        auto answer = QMessageBox::question(
            this,
            "Kristall",
            tr("You want to visit a new host, but have a client certificate enabled. This may be a risk to expose your identity to another host.\r\nDo you want to keep the certificate enabled?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if(answer != QMessageBox::Yes) {
            this->disableClientCertificate();
        }
    }

    if(this->current_identity.isValid() and this->current_identity.isHostFiltered(url)) {
        auto answer = QMessageBox::question(
            this,
            "Kristall",
            tr("Your client certificate has a host filter enabled and this site does not match the host filter.\r\n"
               "New URL: %1\r\nHost Filter: %2\r\nDo you want to keep the certificate enabled?")
                .arg(url.toString(QUrl::FullyEncoded | QUrl::RemoveFragment), this->current_identity.host_filter),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if(answer != QMessageBox::Yes) {
            this->disableClientCertificate();
        }
    }
    else if(not this->current_identity.isValid()) {
        for(auto ident_ptr : kristall::identities.allIdentities())
        {
            if(ident_ptr->isAutomaticallyEnabledOn(url)) {

                auto answer = QMessageBox::question(
                    this,
                    "Kristall",
                    tr("An automatic client certificate was detected for this site:\r\n%1\r\nDo you want to enable that certificate?")
                        .arg(ident_ptr->display_name),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                );
                if(answer != QMessageBox::Yes) {
                    break;
                }

                enableClientCertificate(*ident_ptr);

                break;
            }
        }
    }

    if(not try_enable_certificate())
        return false;

    this->is_internal_location = (url.scheme() == "about");
    this->current_location = url;
    this->setUrlBarText(url.toString(QUrl::FormattingOptions(QUrl::FullyEncoded)));

    this->network_timeout_timer.start(kristall::options.network_timeout);

    return this->current_handler->startRequest(url.adjusted(QUrl::RemoveFragment), options);
}

void BrowserTab::updateMouseCursor(bool waiting)
{
    if (waiting)
        this->ui->text_browser->setDefaultCursor(Qt::BusyCursor);
    else
        this->ui->text_browser->setDefaultCursor(Qt::ArrowCursor);
}

bool BrowserTab::enableClientCertificate(const CryptoIdentity &ident)
{
    if (not ident.isValid())
    {
        QMessageBox::warning(this, "Kristall", "Failed to generate temporary crypto-identitiy");
        this->disableClientCertificate();
        return false;
    }
    this->current_identity = ident;
    this->ui->enable_client_cert_button->setChecked(true);
    return true;
}

void BrowserTab::disableClientCertificate()
{
    for(auto & handler : this->protocol_handlers) {
        handler->disableClientCertificate();
    }
    this->ui->enable_client_cert_button->setChecked(false);
    this->current_identity = CryptoIdentity();
}

void BrowserTab::on_text_browser_customContextMenuRequested(const QPoint pos)
{
    QMenu menu;

    QString anchor = ui->text_browser->anchorAt(pos);
    if (not anchor.isEmpty())
    {
        QUrl real_url{anchor};
        if (real_url.isRelative())
            real_url = this->current_location.resolved(real_url);

        connect(menu.addAction("Follow link…"), &QAction::triggered, [this, real_url]() {
            this->navigateTo(real_url, PushImmediate);
        });

        connect(menu.addAction("Open in new tab…"), &QAction::triggered, [this, real_url]() {
            mainWindow->addNewTab(false, real_url);
        });

        connect(menu.addAction("Copy link"), &QAction::triggered, [real_url]() {
            kristall::clipboard->setText(real_url.toString(QUrl::FullyEncoded));
        });

        menu.addSeparator();
    }

    menu.addAction(QIcon::fromTheme("go-previous"), tr("Back"), [this]() {
        this->on_back_button_clicked();
    });

    menu.addAction(QIcon::fromTheme("go-next"), tr("Forward"), [this]() {
        this->on_forward_button_clicked();
    });

    menu.addSeparator();

    connect(menu.addAction("Select all"), &QAction::triggered, [this]() {
        this->ui->text_browser->selectAll();
    });

    menu.addSeparator();

    QAction * copy = menu.addAction("Copy to clipboard");
    copy->setShortcut(QKeySequence("Ctrl+C"));
    connect(copy, &QAction::triggered, [this]() {
        this->ui->text_browser->copy();
    });

    menu.addSeparator();

    QAction * viewsrc = menu.addAction("View document source");
    viewsrc->setShortcut(QKeySequence("Ctrl+U"));
    connect(viewsrc, &QAction::triggered, [this]() {
        mainWindow->viewPageSource();
    });

    menu.exec(ui->text_browser->mapToGlobal(pos));
}

void BrowserTab::on_enable_client_cert_button_clicked(bool checked)
{
    if (checked)
    {
        trySetClientCertificate(QString{});
    }
    else
    {
        resetClientCertificate();
    }
}

void BrowserTab::on_search_box_textChanged(const QString &arg1)
{
    this->ui->text_browser->setTextCursor(QTextCursor { this->ui->text_browser->document() });
    this->ui->text_browser->find(arg1);
}

void BrowserTab::on_search_box_returnPressed()
{
    this->ui->text_browser->find(this->ui->search_box->text());
}

void BrowserTab::on_search_next_clicked()
{
    this->ui->text_browser->find(this->ui->search_box->text());
}

void BrowserTab::on_search_previous_clicked()
{
    this->ui->text_browser->find(this->ui->search_box->text(), QTextDocument::FindBackward);
}

void BrowserTab::on_close_search_clicked()
{
    this->ui->search_bar->setVisible(false);
}
