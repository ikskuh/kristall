#include "favouritepopup.hpp"

#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>

FavouritePopup::FavouritePopup(QToolButton *button, QWidget *parent)
    : QMenu(parent), b(button)
{
    this->is_ready = false;

    auto parent_layout = new QVBoxLayout();
    parent_layout->setContentsMargins(8, 8, 8, 8);

    auto layout = new QGridLayout();

    // Title
    auto title_lab = new QLabel(tr("Title:"));
    this->fav_title = new QLineEdit();
    layout->addWidget(title_lab, 0, 0);
    layout->addWidget(this->fav_title, 0, 1);

    // Group
    auto group_lab = new QLabel(tr("Group:"));
    layout->addWidget(group_lab);
    {
        this->fav_group = new QComboBox();

        auto new_group = new QToolButton();
        new_group->setIcon(QIcon::fromTheme("document-new"));
        connect(new_group, &QPushButton::clicked, this, [this]() {
            emit this->newGroupClicked();
        });

        auto group_lay = new QHBoxLayout();
        group_lay->addWidget(this->fav_group);
        group_lay->addWidget(new_group);
        layout->addLayout(group_lay, 1, 1);
    }

    // Unfavourite
    auto unfav_btn = new QPushButton(tr("Unfavourite"));
    layout->addWidget(unfav_btn);
    connect(unfav_btn, &QPushButton::clicked, this, [this]() {
        this->setVisible(false);
        emit this->unfavourited();
    });

    // Confirm
    this->confirm_btn = new QPushButton(tr("Confirm"));
    layout->addWidget(this->confirm_btn);
    connect(confirm_btn, &QPushButton::clicked, this, [this]() {
        this->confirmPressed();
    });

    parent_layout->addLayout(layout);

    this->setLayout(parent_layout);
    this->setMinimumWidth(350);
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
