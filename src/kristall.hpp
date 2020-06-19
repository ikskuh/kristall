#ifndef KRISTALL_HPP
#define KRISTALL_HPP

#include <QSettings>
#include <QClipboard>

#include "identitycollection.hpp"
#include "ssltrust.hpp"
#include "favouritecollection.hpp"

enum class Theme : int
{
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

extern QSettings global_settings;
extern IdentityCollection global_identities;
extern QClipboard * global_clipboard;
extern SslTrust global_gemini_trust;
extern SslTrust global_https_trust;
extern FavouriteCollection global_favourites;
extern GenericSettings global_options;

#endif // KRISTALL_HPP
