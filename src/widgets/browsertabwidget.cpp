#include "browsertabwidget.hpp"

#include "browsertabbar.hpp"

BrowserTabWidget::BrowserTabWidget(QWidget *parent) : QTabWidget(parent)
{
    this->setTabBar(new BrowserTabBar(this));
}
