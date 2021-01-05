#include "favouritebutton.hpp"

#include <QStylePainter>
#include <QStyleOptionToolButton>

FavouriteButton::FavouriteButton(QWidget *parent)
    : QToolButton(parent)
{}

void FavouriteButton::paintEvent(QPaintEvent *event)
{
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.features &= (~QStyleOptionToolButton::HasMenu);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}
