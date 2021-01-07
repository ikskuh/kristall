#include "kristalltextbrowser.hpp"

#include <QMouseEvent>
#include <QScroller>
#include <QTouchDevice>

KristallTextBrowser::KristallTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    connect(this, &QTextBrowser::anchorClicked, this, &KristallTextBrowser::on_anchorClicked);

    // Enable touch scrolling
    if (QTouchDevice::devices().length() > 0)
    {
        this->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
        QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    }
}

void KristallTextBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        // Fake a middle-click event here
        QMouseEvent fake_event {
            event->type(),
            event->localPos(),
            Qt::LeftButton,
            event->buttons(),
            event->modifiers()
        };

        this->signal_new_tab = true;
        QTextBrowser::mouseReleaseEvent(&fake_event);
    } else {
        this->signal_new_tab = event->modifiers().testFlag(Qt::ControlModifier);
        QTextBrowser::mouseReleaseEvent(event);
    }
}

void KristallTextBrowser::on_anchorClicked(const QUrl &url)
{
    emit this->anchorClicked(url, this->signal_new_tab);
}


void KristallTextBrowser::mouseMoveEvent(QMouseEvent *event)
{
    QTextBrowser::mouseMoveEvent(event);
    this->updateCursor();
}

void KristallTextBrowser::setDefaultCursor(const QCursor &cur)
{
    this->wanted_cursor = cur;
    this->updateCursor();
}

void KristallTextBrowser::focusInEvent(QFocusEvent *event)
{
    QTextBrowser::focusInEvent(event);
    this->updateCursor();
}

void KristallTextBrowser::updateCursor()
{
    // This slight hack allows us to set the "default" cursor,
    // (i.e when not hovering over links) We need to do this
    // because QTextBrowser for some reason resets viewport cursor
    // to ArrowCursor after we hover over links
    const QCursor& cur = this->viewport()->cursor();
    if (cur.shape() != this->wanted_cursor.shape() &&
        cur.shape() != Qt::PointingHandCursor)
    {
        this->viewport()->setCursor(wanted_cursor);
    }
}
