#include "mainwindow.hpp"
#include "kristall.hpp"

#include <QApplication>
#include <QTranslator>
#include <QUrl>
#include <QSettings>
#include <QCommandLineParser>
#include <QDebug>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QLocalSocket>
#include <QLocalServer>
#include <QLibraryInfo>
#include <cassert>

ProtocolSetup       kristall::protocols;
IdentityCollection  kristall::identities;
QSettings *         kristall::settings;
QClipboard *        kristall::clipboard;
SslTrust            kristall::trust::gemini;
SslTrust            kristall::trust::https;
FavouriteCollection kristall::favourites;
GenericSettings     kristall::options;
DocumentStyle       kristall::document_style(false);
CacheHandler        kristall::cache;
QString             kristall::default_font_family;
QString             kristall::default_font_family_fixed;

QDir kristall::dirs::config_root;
QDir kristall::dirs::cache_root;
QDir kristall::dirs::offline_pages;
QDir kristall::dirs::themes;
QDir kristall::dirs::styles;

// We need QFont::setFamilies for emojis to work properly,
// Qt versions below 5.13 don't support this.
const bool kristall::EMOJIS_SUPPORTED =
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    false;
#else
    true;
#endif

QString toFingerprintString(QSslCertificate const & certificate)
{
    return QCryptographicHash::hash(certificate.toDer(), QCryptographicHash::Sha256).toHex(':');
}

char const * const app_window_property = "kristall:app-window";

static QSettings * app_settings_ptr= nullptr;
static MainWindow * last_focused_window = nullptr;
static bool closing_state_saved = false;

#define SSTR(X) STR(X)
#define STR(X) #X

static QDir derive_dir(QDir const & parent, QString const & subdir)
{
    QDir child = parent;
    if(not child.mkpath(subdir)) {
        qWarning() << "failed to initialize directory:" << subdir;
        return QDir { };
    }
    if(not child.cd(subdir)) {
        qWarning() << "failed to setup directory:" << subdir;
        return QDir { };
    }
    return child;
}

static void addEmojiSubstitutions()
{
    QFontDatabase db;

    auto const families = db.families();

    // Provide OpenMoji font for a safe fallback
    QFontDatabase::addApplicationFont(":/fonts/OpenMoji-Color.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoColorEmoji.ttf");

    QStringList emojiFonts = {
        // Use system fonts on windows/mac
        "Apple Color Emoji",
        "Segoe UI Emoji",

        // Provide common fonts as a fallback:
        // "Noto Color Emoji", // this font seems to replace a lot of text characters?
        // "JoyPixels", // this font seems to replace a lot of text characters?

        // Built-in font fallback
        "OpenMoji",
    };

    for(auto const & family: families)
    {
        auto current = QFont::substitutes(family);
        current << emojiFonts;
        // TODO: QFont::insertSubstitutions(family, current);
    }
}

template<typename F>
void forAllAppWindows(F const & f)
{
    assert(qApp != nullptr);
    for(auto * const widget : qApp->topLevelWidgets())
    {
        auto * const main_window = qobject_cast<MainWindow*>(widget);
        if(main_window != nullptr) {
            f(main_window);
        }
    }
}

MainWindow * getFocusedAppWindow()
{
    assert(qApp != nullptr);
    // first, check if we have currently active window:
    auto * main_window = qobject_cast<MainWindow *>(qApp->activeWindow());
    if(main_window != nullptr)
        return main_window;

    // if not, fall back to the window we focused last:
    if(::last_focused_window != nullptr)
        return ::last_focused_window;

    // and if we have none, we just take the first window we can find if
    // any:
    forAllAppWindows([&main_window](MainWindow * w) {
        if(main_window == nullptr)
            main_window = w;
    });
    if(main_window != nullptr)
        return main_window;

    qCritical() << "could not find a focused/foreground window!";

    return main_window;
}


// Explanation to the IPC protocol:
// Each IPC request is required to open a new connection.
// The first some bytes are the ipc::Message struct that is inspected.
// After that, message-dependent bytes are sent and the IPC process is terminated.
// There is now way of indicating an error.
// Messages are not allowed to be larger than 64k
// We don't need to think about endianess or alignment as we only communicate with
// the same machine.
namespace ipc
{
    static char const * socket_name = "net.random-projects.kristall";

    struct Message
    {
        enum Type
        {
            /// Requests that a series of urls URLs are opened
            /// as new tabs in the currently focused window.
            /// Payload description:
            /// The message will contain a sequence of UTF-8 encoded bytes
            /// that encode URLs. The URLs are separated by LF.
            open_in_tabs = 0,

            /// Same format as open_in_tabs, but requests that these urls are
            /// opened in a new window instead of new tabs.
            open_in_window = 1,
        };

        enum Protocol : uint16_t
        {
            version_1 = 1,
        };

        Protocol version;
        Type type;
    };

    static_assert(std::is_trivial_v<Message>, "Message needs to be flat-copyable!");
    static_assert(std::is_trivially_copyable_v<Message>, "Message needs to be flat-copyable!");

    //! Implements the
    struct ConnectedClient : QObject
    {
        QLocalSocket * socket;
        QByteArray receive_buffer;
        bool everything_ok;

        ConnectedClient(QLocalSocket * socket) :
            QObject(socket),
            socket(socket),
            everything_ok(true)
        {
            QObject::connect(socket, &QLocalSocket::readyRead, this, &ConnectedClient::on_readyRead);
            QObject::connect(socket, &QLocalSocket::disconnected, this, &ConnectedClient::on_disconnected);
        }

        void on_readyRead()
        {
            auto const buffer = socket->readAll();
            if(buffer.size() + this->receive_buffer.size() > 65536) {
                qCritical() << "ipc failure: IPC client sent more than 64k bytes of data!";
                this->everything_ok = false;
                this->socket->close();
            }

            this->receive_buffer.append(buffer);
        }

        void on_disconnected()
        {
            if(not this->everything_ok)
                return;
            if(size_t(this->receive_buffer.size()) < sizeof(Message)) {
                qCritical() << "ipc failure: IPC client did not send enough data!";
                return;
            }
            Message message;
            memcpy(&message, this->receive_buffer.data(), sizeof(Message));
            switch(message.version)
            {
            case Message::version_1: {
                this->processRequest(
                    message.type,
                    this->receive_buffer.mid(sizeof(Message))
                );
                break;
            }
            default: {
                qCritical() << "ipc failure: IPC client used a unsupported protocol version!";
                return;
            }
            }
        }

        void processRequest(Message::Type type, QByteArray const  & payload)
        {
            switch(type)
            {
            case Message::open_in_tabs: {
                auto * const window = getFocusedAppWindow();
                for(auto const & data : payload.split('\n'))
                {
                    QUrl url { QString::fromUtf8(data) };
                    if(url.isValid()) {
                        if(window != nullptr) {
                            window->addNewTab(true, url);
                        }
                    }
                }
                break;
            }
            case Message::open_in_window: {
                QVector<QUrl> urls;
                for(auto const & data : payload.split('\n'))
                {
                    QUrl url { QString::fromUtf8(data) };
                    if(url.isValid()) {
                        urls.append(url);
                    }
                }
                if(urls.size() > 0) {
                    kristall::openNewWindow(urls);
                }
                break;
            }

            default: {
                qCritical() << "ipc failure: IPC client used a unsupported message type!";
                return;
            }
            }
        }
    };

    void send(QLocalSocket & socket, void const * buffer, size_t length)
    {
        size_t offset = 0;
        while(offset < length)
        {
            auto const sent = socket.write(
                reinterpret_cast<char const *>(buffer) + offset,
                length - offset
            );
            if(sent <= 0)
                return;
            offset += sent;
        }
    }

    /// Sends a open_in_tabs request and closes the socket.
    void sendOpenInTabs(QLocalSocket & socket, QVector<QUrl> const & urls)
    {
        Message msg { Message::version_1, Message::open_in_tabs };
        send(socket, &msg, sizeof msg);
        for(int i = 0; i < urls.size(); i++)
        {
            if(i > 0)
                send(socket, "\n", 1);
            auto const bits = urls[i].toString(QUrl::FullyEncoded).toUtf8();
            send(socket, bits.data(), bits.size());
        }
    }

    /// Sends a open_in_window request and closes the socket.
    void sendOpenInWindow(QLocalSocket & socket, QVector<QUrl> const & urls)
    {
        Message msg { Message::version_1, Message::open_in_window };
        send(socket, &msg, sizeof msg);
        for(int i = 0; i < urls.size(); i++)
        {
            if(i > 0)
                send(socket, "\n", 1);
            auto const bits = urls[i].toString(QUrl::FullyEncoded).toUtf8();
            send(socket, bits.data(), bits.size());
        }
    }
}

void kristall::registerAppWindow(MainWindow * window)
{
    window->setProperty(app_window_property, QVariant(true));
    QObject::connect(window, &MainWindow::destroyed, [window]() {
        // reset the focus so we don't store a dangling pointer!
        if(::last_focused_window == window)
            ::last_focused_window = nullptr;
    });
}

MainWindow * kristall::openNewWindow(bool load_default)
{
    auto * const window = openNewWindow(QVector<QUrl>{});
    window->addEmptyTab(true, load_default);
    return window;
}

//! Opens a new window with the given url.
MainWindow * kristall::openNewWindow(QUrl const & url)
{
    return openNewWindow(QVector<QUrl>{url});
}

//! Opens a new window with the given list of urls.
//! If the list is empty, no new tab will spawned.
MainWindow * kristall::openNewWindow(QVector<QUrl> const & urls)
{
    MainWindow * const window = new MainWindow(qApp);

    for(int i = 0; i < urls.length(); i++)
    {
        window->addNewTab((i == 0), urls.at(i));
    }

    window->show();

    return window;
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(SSTR(KRISTALL_VERSION));

    QObject::connect(&app, &QApplication::focusChanged, [](QWidget *old, QWidget *now) {
        // Determine the window for both, we're only interested in window focus changes.
        if(old != nullptr) old = old->window();
        if(now != nullptr) now = now->window();

        if(old == now) // no focus change here
            return;

        auto * const main_window = qobject_cast<MainWindow*>(now);
        if(main_window != nullptr) {

            auto is_app_window = main_window->property(app_window_property);
            if(is_app_window.toBool() == true) {
                // safety measure to not accidently store non-resettable pointers here
                ::last_focused_window = main_window;
            }
            else {
                qCritical() << main_window << "is not registered as a proper app window!";
            }
        }
    });

    QTranslator trans, qttrans;
    qttrans.load(QLocale(), QLatin1String("qt"), "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    trans.load(QLocale(), QLatin1String("kristall"), QLatin1String("_"), QLatin1String(":/i18n"));
    app.installTranslator(&qttrans);
    app.installTranslator(&trans);

    {
        // Initialise default fonts
    #ifdef Q_OS_WIN32
        // Windows default fonts are ugly, so we use standard ones.
        kristall::default_font_family = "Segoe UI";
        kristall::default_font_family_fixed = "Consolas";
    #else
        // *nix
        kristall::default_font_family = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
        kristall::default_font_family_fixed = QFontInfo(QFont("monospace")).family();
    #endif
        kristall::document_style.initialiseDefaultFonts();
    }

    kristall::clipboard = app.clipboard();

    addEmojiSubstitutions();

    QCommandLineParser cli_parser;

    QCommandLineOption newWindowOption {
        { "w", "new-window" },
        QApplication::tr("Opens the provided links in a new window instead of tabs."),
    };

    QCommandLineOption isolatedOption {
        { "i", "isolated" },
        QApplication::tr("Starts the instance of kristall as a isolated session that cannot communicate with other windows."),
    };

    cli_parser.addVersionOption();
    cli_parser.addHelpOption();
    cli_parser.addOption(newWindowOption);
    cli_parser.addOption(isolatedOption);

    cli_parser.addPositionalArgument("urls", QApplication::tr("The urls that should be opened instead of the start page"), "[urls...]");

    cli_parser.process(app);

    QVector<QUrl> urls;
    {
        auto cli_args = cli_parser.positionalArguments();
        for(auto const & url_str : cli_args)
        {
            QUrl url { url_str };
            if (url.isRelative())
            {
                if (QFile::exists(url_str)) {
                    url = QUrl::fromLocalFile(QFileInfo(url_str).absoluteFilePath());
                } else {
                    url = QUrl("gemini://" + url_str);
                }
            }
            if(url.isValid()) {
                urls.append(url);
            } else {
                qDebug() << "Invalid url: " << url_str;
            }
        }
    }

    auto const open_new_window = cli_parser.isSet(newWindowOption);

    auto const isolated_session = cli_parser.isSet(isolatedOption);

    std::unique_ptr<QLocalServer> ipc_server { nullptr };

    if(not isolated_session)
    {
        // try connecting to a already existing instance of kristall
        {
            QLocalSocket socket;
            socket.connectToServer(ipc::socket_name);
            // do not use less time here as we need to give the other task a bit
            // of time. Most OS have a "loop time" of ~10 ms, so we use twice the
            // time here to allow a response.
            if(socket.waitForConnected(20))
            {
                qDebug() << "we already have a kristall instance running!";
                if(urls.length() > 0)
                {
                    if(open_new_window)
                        ipc::sendOpenInWindow(socket, urls);
                    else
                        ipc::sendOpenInTabs(socket, urls);
                }
                socket.waitForBytesWritten();
                return 0;
            }
        }

        // Otherwise, spawn a new local socket that will accept messages
        // to provide proper IPC
        {
            std::unique_ptr<QLocalServer> server { new QLocalServer };
            server->setSocketOptions(QLocalServer::UserAccessOption);
            if(server->listen(ipc::socket_name))
            {
                qDebug() << "successfully started the IPC socket.";
                ipc_server = std::move(server);
            }
            else
            {
                qCritical() << "failed to create IPC socket: " << server->errorString();
            }
        }
    }

    QString cache_root = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString config_root = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    kristall::dirs::config_root = QDir { config_root };
    kristall::dirs::cache_root  = QDir { cache_root };

    kristall::dirs::offline_pages = derive_dir(kristall::dirs::cache_root, "offline-pages");
    kristall::dirs::themes = derive_dir(kristall::dirs::config_root, "themes");

    kristall::dirs::styles = derive_dir(kristall::dirs::config_root, "styles");
    kristall::dirs::styles.setNameFilters(QStringList { "*.kthm" });
    kristall::dirs::styles.setFilter(QDir::Files);

    QSettings app_settings {
        kristall::dirs::config_root.absoluteFilePath("config.ini"),
        QSettings::IniFormat
    };
    app_settings_ptr = &app_settings;

    {
        QSettings deprecated_settings { "xqTechnologies", "Kristall" };
        if(QFile(deprecated_settings.fileName()).exists())
        {
            if(deprecated_settings.value("deprecated", false) == false)
            {
                qDebug() << "Migrating to new configuration style.";
                for(auto const & key : deprecated_settings.allKeys())
                {
                    app_settings.setValue(key, deprecated_settings.value(key));
                }

                // Migrate themes to new model
                {
                    int items = deprecated_settings.beginReadArray("Themes");

                    for(int i = 0; i < items; i++)
                    {
                        deprecated_settings.setArrayIndex(i);

                        QString name = deprecated_settings.value("name").toString();

                        DocumentStyle style;
                        style.load(deprecated_settings);

                        QString fileName;
                        int index = 0;
                        do {
                            fileName = DocumentStyle::createFileNameFromName(name, index);
                            index += 1;
                        } while(kristall::dirs::styles.exists(fileName));

                        QSettings style_sheet {
                            kristall::dirs::styles.absoluteFilePath(fileName),
                                    QSettings::IniFormat
                        };
                        style_sheet.setValue("name", name);
                        style.save(style_sheet);
                        style_sheet.sync();
                    }

                    deprecated_settings.endArray();
                }

                // Remove old theming stuff
                app_settings.remove("Theme");
                app_settings.remove("Themes");

                // Migrate "current theme" to new format
                {
                    DocumentStyle current_style;
                    deprecated_settings.beginGroup("Theme");
                    current_style.load(deprecated_settings);
                    deprecated_settings.endGroup();

                    app_settings.beginGroup("Theme");
                    current_style.save(app_settings);
                    app_settings.endGroup();
                }

                deprecated_settings.setValue("deprecated", true);
            }
            else
            {
                qDebug() << "Migration complete. Please delete" << deprecated_settings.fileName();
            }
        }
    }

    // Migrate to new favourites format
    if(int len = app_settings.beginReadArray("favourites"); len > 0)
    {
        qDebug() << "Migrating old-style favourites...";
        std::vector<Favourite> favs;

        favs.reserve(len);
        for(int i = 0; i < len; i++)
        {
            app_settings.setArrayIndex(i);

            Favourite fav;
            fav.destination = app_settings.value("url").toString();
            fav.title = QString { };

            favs.emplace_back(std::move(fav));
        }
        app_settings.endArray();


        app_settings.beginGroup("Favourites");
        {
            app_settings.beginWriteArray("groups");

            app_settings.setArrayIndex(0);
            app_settings.setValue("name", QObject::tr("Unsorted"));

            {
                app_settings.beginWriteArray("favourites", len);
                for(int i = 0; i < len; i++)
                {
                    auto const & fav = favs.at(i);
                    app_settings.setArrayIndex(i);
                    app_settings.setValue("title", fav.title);
                    app_settings.setValue("url", fav.destination);
                }
                app_settings.endArray();
            }

            app_settings.endArray();
        }
        app_settings.endGroup();

        app_settings.remove("favourites");
    }
    else {
        app_settings.endArray();
    }

    kristall::settings = &app_settings;

    kristall::options.load(app_settings);

    app_settings.beginGroup("Protocols");
    kristall::protocols.load(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Client Identities");
    kristall::identities.load(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Trusted Servers");
    kristall::trust::gemini.load(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Trusted HTTPS Servers");
    kristall::trust::https.load(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Theme");
    kristall::document_style.load(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Favourites");
    kristall::favourites.load(app_settings);
    app_settings.endGroup();

    kristall::setTheme(kristall::options.theme);

    if(ipc_server != nullptr) {
        QObject::connect(ipc_server.get(), &QLocalServer::newConnection, [&ipc_server]() {
            auto * const socket = ipc_server->nextPendingConnection();
            if(socket != nullptr) {
                // this will set up everything needed:
                // - signals from socket
                // - set itself as the socket child, so it will be deleted when the socket is closed
                (void) new ipc::ConnectedClient(socket);

                // destroy the socket when the connection was closed.
                QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            }
        });
    }

    // Open all URLs in the new window
    if(urls.size() > 0) {
        kristall::openNewWindow(urls);
    }
    else {
        kristall::openNewWindow(true);
    }

    //app_settings.beginGroup("Window State");
    //if(app_settings.contains("geometry")) {
    //    w.restoreGeometry(app_settings.value("geometry").toByteArray());
    //}
    //if(app_settings.contains("state")) {
    //    w.restoreState(app_settings.value("state").toByteArray());
    //}
    //app_settings.endGroup();

    int exit_code = app.exec();

    if (!closing_state_saved)
        kristall::saveWindowState();

    return exit_code;
}

void GenericSettings::load(QSettings &settings)
{
    network_timeout = settings.value("network_timeout", 5000).toInt();
    start_page = settings.value("start_page", "about:favourites").toString();
    search_engine = settings.value("search_engine", "gemini://geminispace.info/search?%1").toString();

    if(settings.value("text_display", "fancy").toString() == "plain")
        text_display = PlainText;
    else
        text_display = FormattedText;

    enable_text_decoration = settings.value("text_decoration", false).toBool();

    QString theme_name = settings.value("theme", "os_default").toString();
    if(theme_name == "dark")
        theme = Theme::dark;
    else if(theme_name == "light")
        theme = Theme::light;
    else
        theme = Theme::os_default;

    QString icon_theme_name = settings.value("icon_theme", "auto").toString();
    if (icon_theme_name == "light")
        icon_theme = IconTheme::light;
    else if (icon_theme_name == "dark")
        icon_theme = IconTheme::dark;
    else
        icon_theme = IconTheme::automatic;

    QString density = settings.value("ui_density", "compact").toString();
    if(density == "compact")
        ui_density = UIDensity::compact;
    else if (density == "classic")
        ui_density = UIDensity::classic;

    if(settings.value("gophermap_display", "rendered").toString() == "rendered")
        gophermap_display = FormattedText;
    else
        gophermap_display = PlainText;

    use_os_scheme_handler = settings.value("use_os_scheme_handler", false).toBool();

    show_hidden_files_in_dirs = settings.value("show_hidden_files_in_dirs", false).toBool();

    fancy_urlbar = settings.value("fancy_urlbar", true).toBool();

    fancy_quotes = settings.value("fancy_quotes", true).toBool();

    emojis_enabled = kristall::EMOJIS_SUPPORTED
        ? settings.value("emojis_enabled", true).toBool()
        : false;

    max_redirections = settings.value("max_redirections", 5).toInt();
    redirection_policy = RedirectionWarning(settings.value("redirection_policy ", WarnOnHostChange).toInt());

    enable_home_btn = settings.value("enable_home_btn", false).toBool();
    enable_newtab_btn = settings.value("enable_newtab_btn", true).toBool();
    enable_root_btn = settings.value("enable_root_btn", false).toBool();
    enable_parent_btn = settings.value("enable_parent_btn", false).toBool();

    cache_limit = settings.value("cache_limit", 1000).toInt();
    cache_threshold = settings.value("cache_threshold", 125).toInt();
    cache_life = settings.value("cache_life", 15).toInt();
    cache_unlimited_life = settings.value("cache_unlimited_life", true).toBool();
}

void GenericSettings::save(QSettings &settings) const
{
    settings.setValue("start_page", this->start_page);
    settings.setValue("search_engine", this->search_engine);
    settings.setValue("text_display", (text_display == FormattedText) ? "fancy" : "plain");
    settings.setValue("text_decoration", enable_text_decoration);
    QString theme_name = "os_default";
    switch(theme) {
    case Theme::dark:       theme_name = "dark"; break;
    case Theme::light:      theme_name = "light"; break;
    case Theme::os_default: theme_name = "os_default"; break;
    }
    settings.setValue("theme", theme_name);

    QString icon_theme_name = "auto";
    switch(icon_theme) {
    case IconTheme::dark:      icon_theme_name = "dark"; break;
    case IconTheme::light:     icon_theme_name = "light"; break;
    case IconTheme::automatic: icon_theme_name = "auto"; break;
    }
    settings.setValue("icon_theme", icon_theme_name);

    QString density = "compact";
    switch(ui_density) {
    case UIDensity::compact: density = "compact"; break;
    case UIDensity::classic: density = "classic"; break;
    }
    settings.setValue("ui_density", density);

    settings.setValue("gophermap_display", (gophermap_display == FormattedText) ? "rendered" : "text");
    settings.setValue("use_os_scheme_handler", use_os_scheme_handler);
    settings.setValue("show_hidden_files_in_dirs", show_hidden_files_in_dirs);
    settings.setValue("fancy_urlbar", fancy_urlbar);
    settings.setValue("fancy_quotes", fancy_quotes);
    settings.setValue("max_redirections", max_redirections);
    settings.setValue("redirection_policy", int(redirection_policy));
    settings.setValue("network_timeout", network_timeout);
    settings.setValue("enable_home_btn", enable_home_btn);
    settings.setValue("enable_newtab_btn", enable_newtab_btn);
    settings.setValue("enable_root_btn", enable_root_btn);
    settings.setValue("enable_parent_btn", enable_parent_btn);

    settings.setValue("cache_limit", cache_limit);
    settings.setValue("cache_threshold", cache_threshold);
    settings.setValue("cache_life", cache_life);
    settings.setValue("cache_unlimited_life", cache_unlimited_life);

    if (kristall::EMOJIS_SUPPORTED)
    {
        // Save emoji pref only if emojis are supported, so if user changes to a build
        // with emoji support, they get it out of the box.
        settings.setValue("emojis_enabled", emojis_enabled);
    }
}

void kristall::applySettings()
{
    kristall::setTheme(kristall::options.theme);
    kristall::setUiDensity(kristall::options.ui_density, false);

    forAllAppWindows([](MainWindow * window)
    {
        window->applySettings();
    });
}

void kristall::saveSettings()
{
    assert(app_settings_ptr != nullptr);
    QSettings & app_settings = *app_settings_ptr;

    app_settings.beginGroup("Favourites");
    kristall::favourites.save(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Protocols");
    kristall::protocols.save(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Client Identities");
    kristall::identities.save(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Trusted Servers");
    kristall::trust::gemini.save(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Trusted HTTPS Servers");
    kristall::trust::https.save(app_settings);
    app_settings.endGroup();

    app_settings.beginGroup("Theme");
    kristall::document_style.save(app_settings);
    app_settings.endGroup();

    kristall::options.save(app_settings);

    app_settings.sync();
}

void kristall::setTheme(Theme theme)
{
    assert(qApp != nullptr);

    if(theme == Theme::os_default)
    {
        qApp->setStyleSheet("");

        // Use "mid" colour for our URL bar dim colour:
        QColor col = qApp->palette().color(QPalette::WindowText);
        col.setAlpha(150);
        kristall::options.fancy_urlbar_dim_colour = std::move(col);
    }
    else if(theme == Theme::light)
    {
        QFile file(":/light.qss");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());

        kristall::options.fancy_urlbar_dim_colour = QColor(128, 128, 128, 255);
    }
    else if(theme == Theme::dark)
    {
        QFile file(":/dark.qss");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());

        kristall::options.fancy_urlbar_dim_colour = QColor(150, 150, 150, 255);
    }

    kristall::setIconTheme(kristall::options.icon_theme, theme);

    forAllAppWindows([](MainWindow * main_window) {
        if (main_window && main_window->curTab())
            main_window->curTab()->updateUrlBarStyle();
    });
}

void kristall::setIconTheme(IconTheme icotheme, Theme uitheme)
{
    assert(qApp != nullptr);

    static const QString icothemes[] = {
        "light", // Light theme (dark icons)
        "dark"   // Dark theme (light icons)
    };

    auto ret = []() {
        forAllAppWindows([](MainWindow * main_window) {
            if (main_window && main_window->curTab())
                main_window->curTab()->refreshToolbarIcons();
        });
    };

    if (icotheme == IconTheme::automatic)
    {
        if (uitheme == Theme::os_default)
        {
            // For Linux we use standard system icon set,
            // for Windows & Mac we just use our default light theme icons.
        #if defined Q_OS_WIN32 || defined Q_OS_DARWIN
            QIcon::setThemeName("light");
        #else
            QIcon::setThemeName("");
        #endif

            kristall::options.explicit_icon_theme = IconTheme::dark;

            ret();
            return;
        }

        // Use icon theme based on UI theme
        QIcon::setThemeName(icothemes[(int)uitheme]);
        kristall::options.explicit_icon_theme = (IconTheme)uitheme;
        ret();
        return;
    }

    // Use icon specified by user
    QIcon::setThemeName(icothemes[(int)icotheme]);
    kristall::options.explicit_icon_theme = (IconTheme)icotheme;
    ret();
}

void kristall::setUiDensity(UIDensity density, bool previewing)
{
    forAllAppWindows([density, previewing](MainWindow * main_window) {
        main_window->setUiDensity(density, previewing);
    });
}

void kristall::saveWindowState()
{
    closing_state_saved = true;

    app_settings_ptr->beginGroup("Window State");
    //app_settings_ptr->setValue("geometry", main_window->saveGeometry());
    //app_settings_ptr->setValue("state", main_window->saveState());
    app_settings_ptr->endGroup();

    kristall::saveSettings();
}
