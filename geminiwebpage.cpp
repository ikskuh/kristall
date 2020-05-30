#include "geminiwebpage.hpp"
#include "browsertab.hpp"

GeminiWebPage::GeminiWebPage(MainWindow * container) :
    QWebEnginePage(),
    main_window(container)
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

QWebEnginePage *GeminiWebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    auto tab = main_window->addEmptyTab(true);

    return &tab->page;
}
