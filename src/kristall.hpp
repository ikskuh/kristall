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

    QString start_page = "about:favourites";
    Theme theme = Theme::light;
    UIDensity ui_density = UIDensity::compact;
    TextDisplay text_display = FormattedText;
    bool enable_text_decoration = false;
    bool use_os_scheme_handler = false;
    bool show_hidden_files_in_dirs = false;
    bool fancy_urlbar = true;

    // This is set automatically
    QColor fancy_urlbar_dim_colour;

    TextDisplay gophermap_display = FormattedText;
    int max_redirections = 5;
    RedirectionWarning redirection_policy = WarnOnHostChange;

    // 5 seconds network timeout
    int network_timeout = 5000;

    // Additional toolbar items
    bool enable_home_btn = false;

    // In-memory caching
    int cache_limit = 1000;
    int cache_threshold = 125;
    int cache_life = 15;

    void load(QSettings & settings);
    void save(QSettings & settings) const;
};

//! Converts the certificate to a standardized fingerprint representation
//! also commonly used in browsers:
//! `:`-separated SHA256 hash
QString toFingerprintString(QSslCertificate const & certificate);

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
    extern ProtocolSetup protocols;

    extern QSettings * settings;
    extern IdentityCollection identities;
    extern QClipboard * clipboard;
    extern FavouriteCollection favourites;
    extern GenericSettings options;

    extern DocumentStyle document_style;

    extern CacheHandler cache;

    namespace trust {
        extern SslTrust gemini;
        extern SslTrust https;
    }

    namespace dirs {
        extern QDir config_root;
        extern QDir cache_root;

        //! Contains files per host
        extern QDir offline_pages;

        //! Contains custom UI themes for kristall
        extern QDir themes;

        //! Contains custom document styles / presets
        extern QDir styles;
    }

    void saveSettings();

    void setTheme(Theme theme);

    void setUiDensity(UIDensity density, bool previewing);

    extern QString default_font_family, default_font_family_fixed;
}

#endif // KRISTALL_HPP
