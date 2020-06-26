#include "browsertabbar.hpp"

#include <QMouseEvent>

BrowserTabBar::BrowserTabBar(QWidget *parent) :
    QTabBar(parent)
{

}

void BrowserTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        emit tabCloseRequested(this->tabAt(event->pos()));
    } else {
        QTabBar::mousePressEvent(event);
    }
}
