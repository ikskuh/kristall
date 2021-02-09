#ifndef SEARCHBOX_HPP
#define SEARCHBOX_HPP

#include <QLineEdit>

class SearchBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBox(QWidget *parent = nullptr);

signals:
    void searchNext();
    void searchPrev();
public:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // SEARCHBOX_HPP
