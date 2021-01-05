#ifndef FAVOURITE_BUTTON_HPP
#define FAVOURITE_BUTTON_HPP

//! This class is used to implement a workaround
//! to get rid of the "little arrow" that Qt
//! forces onto toolbuttons that have menus.
//!
//! https://bugreports.qt.io/browse/QTBUG-2036

#include <QToolButton>
#include <QtGui>

class FavouriteButton : public QToolButton
{
    Q_OBJECT;
public:
    explicit FavouriteButton(QWidget * parent = 0);

protected:
    virtual void paintEvent(QPaintEvent * event);

};


#endif
