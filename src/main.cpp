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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    global_clipboard = app.clipboard();

    QCommandLineParser cli_parser;
    cli_parser.parse(app.arguments());

    if(not global_settings.contains("start_page")) {
        global_settings.setValue("start_page", "about:favourites");
    }

    global_settings.beginGroup("Client Identities");
    global_identities.load(global_settings);
    global_settings.endGroup();

    global_settings.beginGroup("Trusted Servers");
    global_trust.load(global_settings);
    global_settings.endGroup();

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
