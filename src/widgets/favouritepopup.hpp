#ifndef FAVOURITEPOPUP_HPP
#define FAVOURITEPOPUP_HPP

#include <QMenu>
#include <QPushButton>
#include <QLineEdit>

class QToolButton;

class FavouritePopup : public QMenu
{
    Q_OBJECT;
public:
    explicit FavouritePopup(QToolButton * button, QWidget * parent);

    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void confirmed();

    void unfavourited();

private:
    void confirmPressed();

private:
    QToolButton *b;

public:
    QLineEdit *fav_title;
    QPushButton *confirm_btn;

};

#endif
