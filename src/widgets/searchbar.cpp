#include "searchbar.hpp"
#include "kristall.hpp"

#include <QKeyEvent>
#include <QCompleter>

SearchBar::SearchBar(QWidget *parent) : QLineEdit(parent)
{
    QCompleter *completer = new QCompleter(this);
    completer->setModel(&kristall::favourites);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionRole(Qt::DisplayRole);
    this->setCompleter(completer);
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
