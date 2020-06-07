#include "mainwindow.hpp"

#include <QApplication>
#include <QUrl>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w(&app);
    w.addEmptyTab(true, true);
    w.show();

    return app.exec();
}
