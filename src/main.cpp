#include "mainwindow.hpp"

#include <QApplication>
#include <QUrl>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // w.addNewTab(true, QUrl("gemini://gemini.circumlunar.space/"));
    w.addEmptyTab(true, true);

    w.show();
    return a.exec();
}
