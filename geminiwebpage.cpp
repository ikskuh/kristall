#include "geminiwebpage.hpp"

GeminiWebPage::GeminiWebPage(QObject *parent) : QWebEnginePage(parent)
{

}

bool GeminiWebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    switch(type)
    {
    // link navigation
    case QWebEnginePage::NavigationTypeLinkClicked: {
        bool result = false;
        emit this->navigationRequest(url, result);
        return result;
    }

    // manual navigation
    case QWebEnginePage::NavigationTypeTyped:
        return true;

    // we do this by hand!
    case QWebEnginePage::NavigationTypeFormSubmitted:
    case QWebEnginePage::NavigationTypeBackForward:
    case QWebEnginePage::NavigationTypeReload:
        return false;

    // forbidden by default
    case QWebEnginePage::NavigationTypeOther:
    case QWebEnginePage::NavigationTypeRedirect:
        return false;
    }
}
