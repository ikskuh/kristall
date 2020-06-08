#include "mainwindow.hpp"

#include <QApplication>
#include <QUrl>
#include <QSettings>
#include <QCommandLineParser>
#include <QDebug>

QSettings global_settings { "xqTechnologies", "Kristall" };

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser cli_parser;
    cli_parser.parse(app.arguments());

    if(not global_settings.contains("start_page")) {
        global_settings.setValue("start_page", "about:favourites");
    }

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
