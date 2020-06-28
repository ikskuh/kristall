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

enum class Theme : int
{
    os_default = -1,
    light = 0,
    dark = 1,
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
    TextDisplay text_display = FormattedText;
    bool enable_text_decoration = false;
    bool use_os_scheme_handler = false;
    TextDisplay gophermap_display = FormattedText;
    int max_redirections = 5;
    RedirectionWarning redirection_policy = WarnOnHostChange;

    // 5 seconds network timeout
    int network_timeout = 5000;

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
}

#endif // KRISTALL_HPP
