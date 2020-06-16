#include "searchbar.hpp"

#include <QKeyEvent>

SearchBar::SearchBar(QWidget *parent) : QLineEdit(parent)
{

}

void SearchBar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        emit this->escapePressed();
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

void SearchBar::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        // Eat the event
    } else {
        QLineEdit::keyReleaseEvent(event);
    }
}
