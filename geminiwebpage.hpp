#ifndef GEMINIWEBPAGE_HPP
#define GEMINIWEBPAGE_HPP

#include <QObject>
#include <QWebEnginePage>

class GeminiWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit GeminiWebPage(QObject *parent = nullptr);


signals:
    void navigationRequest(QUrl const & url, bool & allow);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

};

#endif // GEMINIWEBPAGE_HPP
