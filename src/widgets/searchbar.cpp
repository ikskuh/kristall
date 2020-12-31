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

void SearchBar::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);

    // Allows only one "select all" on mouse release
    // until next focus event.
    this->selectall_flag = (event->reason() == Qt::MouseFocusReason);

    emit this->focused();
}

void SearchBar::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);

    emit this->blurred();
}

void SearchBar::mouseReleaseEvent(QMouseEvent *event)
{
    QLineEdit::mouseReleaseEvent(event);

    // Select all text if the bar was just focused and
    // user did not select anything.
    if (this->selectall_flag && QLineEdit::selectionLength() < 1)
    {
        QLineEdit::selectAll();
    }
    this->selectall_flag = false;
}
