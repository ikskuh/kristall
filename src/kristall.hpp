#ifndef KRISTALL_HPP
#define KRISTALL_HPP

#include <QDir>
#include <QSettings>
#include <QClipboard>
#include <QSslCertificate>

#include "identitycollection.hpp"
#include "ssltrust.hpp"
#include "favouritecollection.hpp"

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

extern QSettings global_settings;
extern IdentityCollection global_identities;
extern QClipboard * global_clipboard;
extern SslTrust global_gemini_trust;
extern SslTrust global_https_trust;
extern FavouriteCollection global_favourites;
extern GenericSettings global_options;

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
    extern QDir config_root;
    extern QDir cache_root;
    extern QDir offline_pages;
    extern QDir themes;
    extern QDir styles;
}

#endif // KRISTALL_HPP
