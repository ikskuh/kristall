#include "favouritepopup.hpp"

#include <QToolButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>

FavouritePopup::FavouritePopup(QToolButton *button, QWidget *parent)
    : QMenu(parent), b(button)
{
    auto parent_layout = new QVBoxLayout();
    parent_layout->setContentsMargins(8, 8, 8, 8);

    auto layout = new QGridLayout();

    // Title
    auto title_lab = new QLabel("Title:");
    this->fav_title = new QLineEdit();
    layout->addWidget(title_lab, 0, 0);
    layout->addWidget(this->fav_title, 0, 1);

    // Unfavourite
    auto unfav_btn = new QPushButton("Unfavourite");
    layout->addWidget(unfav_btn);
    connect(unfav_btn, &QPushButton::clicked, this, [this]() {
        this->setVisible(false);
        emit this->unfavourited();
    });

    // Confirm
    this->confirm_btn = new QPushButton("Confirm");
    layout->addWidget(this->confirm_btn);
    connect(confirm_btn, &QPushButton::clicked, this, [this]() {
        this->confirmPressed();
    });

    parent_layout->addLayout(layout);

    this->setLayout(parent_layout);
    this->setMinimumWidth(250);
}

void FavouritePopup::confirmPressed()
{
    this->setVisible(false);
    emit this->confirmed();
}

void FavouritePopup::showEvent(QShowEvent *event)
{
    QPoint p = this->pos();
    QRect geo = b->geometry();
    this->move(
        p.x() + geo.width() - this->geometry().width(),
        p.y());
}

void FavouritePopup::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        this->confirmPressed();
        return;
    }
    else if (event->key() == Qt::Key_Escape)
    {
        this->setVisible(false);
        return;
    }
    QMenu::keyPressEvent(event);
}

void FavouritePopup::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape)
    {
        QMenu::keyReleaseEvent(event);
    }
}
