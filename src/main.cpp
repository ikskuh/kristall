#include "mainwindow.hpp"
#include "kristall.hpp"

#include <QApplication>
#include <QUrl>
#include <QSettings>
#include <QCommandLineParser>
#include <QDebug>

IdentityCollection global_identities;
QSettings global_settings { "xqTechnologies", "Kristall" };
QClipboard * global_clipboard;
SslTrust global_trust;
FavouriteCollection global_favourites;
GenericSettings global_options;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    global_clipboard = app.clipboard();

    QCommandLineParser cli_parser;
    cli_parser.parse(app.arguments());

    global_options.load(global_settings);

    global_settings.beginGroup("Client Identities");
    global_identities.load(global_settings);
    global_settings.endGroup();

    global_settings.beginGroup("Trusted Servers");
    global_trust.load(global_settings);
    global_settings.endGroup();

    global_favourites.load(global_settings);

    MainWindow w(&app);

    auto urls = cli_parser.positionalArguments();
    if(urls.size() > 0) {
        for(auto url_str : urls) {
            QUrl url { url_str };
            if(url.isValid()) {
                w.addNewTab(false, url);
            } else {
                qDebug() << "Invalid url: " << url_str;
            }
        }
    }
    else {
        w.addEmptyTab(true, true);
    }
    w.show();

    return app.exec();
}

void GenericSettings::load(QSettings &settings)
{
    start_page = settings.value("start_page", "about:favourites").toString();

    if(settings.value("text_display", "fancy").toString() == "plain")
        text_display = PlainText;
    else
        text_display = FormattedText;

    enable_text_decoration = settings.value("text_decoration", false).toBool();

    if(settings.value("theme", "light").toString() == "dark")
        theme = Theme::dark;
    else
        theme = Theme::light;

    if(settings.value("gophermap_display", "rendered").toString() == "rendered")
        gophermap_display = FormattedText;
    else
        gophermap_display = PlainText;

    use_os_scheme_handler = settings.value("use_os_scheme_handler", false).toBool();

    max_redirections = settings.value("max_redirections", 5).toInt();
    redirection_policy = RedirectionWarning(settings.value("redirection_policy ", WarnOnHostChange).toInt());
}

void GenericSettings::save(QSettings &settings) const
{
    settings.setValue("start_page", this->start_page);
    settings.setValue("text_display", (text_display == FormattedText) ? "fancy" : "plain");
    settings.setValue("text_decoration", enable_text_decoration);
    settings.setValue("theme", (theme == Theme::dark) ? "dark" : "light");
    settings.setValue("gophermap_display", (gophermap_display == FormattedText) ? "rendered" : "text");
    settings.setValue("use_os_scheme_handler", use_os_scheme_handler);
    settings.setValue("max_redirections", max_redirections);
    settings.setValue("redirection_policy", int(redirection_policy));
}
