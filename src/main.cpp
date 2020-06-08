#include "mainwindow.hpp"

#include <QApplication>
#include <QUrl>
#include <QSettings>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser cli_parser;
    cli_parser.parse(app.arguments());

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
