#include "browsertabbar.hpp"

#include <QMouseEvent>

static const int NEWTAB_BTN_SIZE = 22,
    NEWTAB_BTN_PAD_X = 4;


BrowserTabBar::BrowserTabBar(QWidget *parent) :
    QTabBar(parent)
{
    newTabBtn = new QPushButton("+", this);
    newTabBtn->setFixedSize(NEWTAB_BTN_SIZE, NEWTAB_BTN_SIZE);
    connect(newTabBtn, &QPushButton::clicked, this, &BrowserTabBar::on_newTabClicked);
    this->newTabBtn->setVisible(true);
}

void BrowserTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        emit tabCloseRequested(this->tabAt(event->pos()));
    } else {
        QTabBar::mousePressEvent(event);
    }
}

void BrowserTabBar::moveNewTabButton()
{
    // Find width of all tabs
    int size = 0;
    for (int i = 0; i < this->count(); ++i)
        size += this->tabRect(i).width();

    // Set location
    int h = this->geometry().top();
    int w = this->width();
    if ((size + NEWTAB_BTN_SIZE + NEWTAB_BTN_PAD_X) > w)
    {
        this->newTabBtn->setVisible(false);
        //this->newTabBtn->move(w - 54, h + 22 / 4);
    }
    else
    {
        this->newTabBtn->setVisible(true);
        this->newTabBtn->move(size + NEWTAB_BTN_PAD_X,
            h + NEWTAB_BTN_SIZE / 4);
    }
}

void BrowserTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    this->moveNewTabButton();
}

void BrowserTabBar::tabLayoutChange()
{
    QTabBar::tabLayoutChange();
    this->moveNewTabButton();
}
