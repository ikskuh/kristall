#ifndef KRISTALL_HPP
#define KRISTALL_HPP

#include <QDir>
#include <QSettings>
#include <QClipboard>
#include <QSslCertificate>

#include "identitycollection.hpp"
#include "ssltrust.hpp"
#include "favouritecollection.hpp"
#include "protocolsetup.hpp"
#include "documentstyle.hpp"
#include "cachehandler.hpp"

enum class Theme : int
{
    os_default = -1,
    light = 0,
    dark = 1,
};

enum class UIDensity : int
{
    compact = 0,
    classic = 1
};

enum class RequestState : int
{
    None = 0,
    Started = 1,
    HostFound = 2,
    Connected = 3,

    StartedWeb = 255,
};

enum class IconTheme : int
{
    automatic = -1,
    dark = 0,
    light = 1
};

enum class AnsiEscRenderMode : int
{
    ignore = 0,
    render = 1,
    strip = 2
};

struct PageMetadata
{
    QUrl location;
    QString title;
};

struct GenericSettings
{
    enum TextDisplay {
        FormattedText,
        PlainText,
    };

    enum RedirectionWarning {
        WarnNever = 0,
        WarnOnSchemeChange = 1,
        WarnOnHostChange = 2,
        WarnAlways = -1,
    };

    enum SessionRestoreBehaviour {
        NoSessionRestore = 0,
        RestoreLastSession = 1,
    };

    QString start_page = "about:favourites";
    QString search_engine = "gemini://geminispace.info/search?%1";
    Theme theme = Theme::light;
    IconTheme icon_theme = IconTheme::automatic;
    IconTheme explicit_icon_theme = IconTheme::dark;
    UIDensity ui_density = UIDensity::compact;
    TextDisplay text_display = FormattedText;
    bool enable_text_decoration = false;
    bool use_os_scheme_handler = false;
    bool show_hidden_files_in_dirs = false;
    bool fancy_urlbar = true;
    bool fancy_quotes = true;
    bool emojis_enabled = true;
    bool strip_nav = false;
    AnsiEscRenderMode ansi_escapes = AnsiEscRenderMode::render;

    // This is set automatically
    QColor fancy_urlbar_dim_colour;

    TextDisplay gophermap_display = FormattedText;
    int max_redirections = 5;
    RedirectionWarning redirection_policy = WarnOnHostChange;

    // 5 seconds network timeout
    int network_timeout = 5000;

    // Additional toolbar items
    bool enable_home_btn = false,
         enable_newtab_btn = true,
         enable_root_btn = false,
         enable_parent_btn = false;

    // In-memory caching
    int cache_limit = 1000;
    int cache_threshold = 125;
    int cache_life = 60;
    bool cache_unlimited_life = true;

    SessionRestoreBehaviour session_restore_behaviour = RestoreLastSession;

    void load(QSettings & settings);
    void save(QSettings & settings) const;
};

//! Converts the certificate to a standardized fingerprint representation
//! also commonly used in browsers:
//! `:`-separated SHA256 hash
QString toFingerprintString(QSslCertificate const & certificate);

// Forward declaration
class MainWindow;

///
/// Kristall directory structure:
/// ~/.cache/kristall/
///     ./offline-pages/${HOST}/${HASHED_URL}
///         : Contains "mime/type\r\n${BLOB}"
/// ~/.config/kristall/
///     ./themes/${THEME_ID}/theme.qss
///     ./styles/${STYLE_ID}.ini
///     ./config.ini
///
namespace kristall
{
    //! Whether emojis are supprted in current build configuration
    extern const bool EMOJIS_SUPPORTED;

    struct Trust
    {
        SslTrust gemini;
        SslTrust https;
    };

    struct Dirs
    {

        QDir config_root;
        QDir cache_root;

        //! Contains files per host
        QDir offline_pages;

        //! Contains custom UI themes for kristall
        QDir themes;

        //! Contains custom document styles / presets
        QDir styles;
    };

    struct Globals
    {

        ProtocolSetup protocols;

        QSettings * settings;
        IdentityCollection identities;
        QClipboard * clipboard;
        FavouriteCollection favourites;
        GenericSettings options;

        DocumentStyle document_style;

        CacheHandler cache;

        Trust trust;

        Dirs dirs;
    };

    //! returns the instance of the globals structure
    Globals & globals();

    //! Forwards the current settings to all windows
    void applySettings();

    //! Saves the current settings to disk
    void saveSettings();

    void setTheme(Theme theme);

    void setIconTheme(IconTheme icotheme, Theme uitheme);

    void setUiDensity(UIDensity density, bool previewing);

    void saveWindowState();

    //! Registers a main window as a proper kristall application window
    //! that can be tracked by the application.
    void registerAppWindow(MainWindow * window);

    //! Opens a new window with the default page open.
    //! @param load_default Loads the default/start page instead of about:blank
    MainWindow * openNewWindow(bool load_default);

    //! Opens a new window with the given url.
    MainWindow * openNewWindow(QUrl const & url);

    //! Opens a new window with the given list of urls.
    //! If the list is empty, no new tab will spawned.
    MainWindow * openNewWindow(QVector<QUrl> const & urls);
    MainWindow * openNewWindow(QVector<PageMetadata> const & urls);

    //! Returns the number of currently open windows
    int getWindowCount();

    //! Saves the current session including all windows, tabs and positions.
    void saveSession();
}

#endif // KRISTALL_HPP
