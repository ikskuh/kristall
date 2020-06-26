#include "kristalltextbrowser.hpp"

#include <QMouseEvent>

KristallTextBrowser::KristallTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{

}

void KristallTextBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    this->last_button = event->button();
    if(event->button() == Qt::MiddleButton) {
        // Fake a middle-click event here
        QMouseEvent fake_event {
            event->type(),
            event->localPos(),
            Qt::LeftButton,
            event->buttons(),
            event->modifiers()
        };

        QTextBrowser::mouseReleaseEvent(&fake_event);
    } else {
        QTextBrowser::mouseReleaseEvent(event);
    }
}
