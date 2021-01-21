#ifndef BROWSERTABWIDGET_HPP
#define BROWSERTABWIDGET_HPP

#include <QTabWidget>
#include "browsertabbar.hpp"

class BrowserTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit BrowserTabWidget(QWidget *parent = nullptr);

public:
    BrowserTabBar *tab_bar;
};

#endif // BROWSERTABWIDGET_HPP
