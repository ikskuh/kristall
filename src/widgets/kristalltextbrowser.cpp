#include "kristalltextbrowser.hpp"

#include <QMouseEvent>

KristallTextBrowser::KristallTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    connect(this, &QTextBrowser::anchorClicked, this, &KristallTextBrowser::on_anchorClicked);
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
