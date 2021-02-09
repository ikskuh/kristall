#include "searchbox.hpp"

#include <QKeyEvent>
#include <QDebug>

SearchBox::SearchBox(QWidget * parent) : QLineEdit(parent)
{}

void SearchBox::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_F3) {
        if (event->modifiers() == Qt::ShiftModifier) {
            emit searchPrev();
        }
        else {
            emit searchNext();
        }
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

void SearchBox::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return) {
        // Eat the event
    } else {
        QLineEdit::keyReleaseEvent(event);
    }
}

