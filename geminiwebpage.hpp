#ifndef GEMINIWEBPAGE_HPP
#define GEMINIWEBPAGE_HPP

#include <QObject>
#include <QWebEnginePage>
#include "mainwindow.hpp"

class GeminiWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit GeminiWebPage(MainWindow * container);


signals:
    void navigationRequest(QUrl const & url, bool & allow);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;

    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    MainWindow * main_window;
};

#endif // GEMINIWEBPAGE_HPP
