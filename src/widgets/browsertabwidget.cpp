#include "browsertabwidget.hpp"

BrowserTabWidget::BrowserTabWidget(QWidget *parent) : QTabWidget(parent)
{
    this->tab_bar = new BrowserTabBar(this);
    this->setTabBar(this->tab_bar);
}
