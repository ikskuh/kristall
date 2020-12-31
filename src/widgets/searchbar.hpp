#ifndef SEARCHBAR_HPP
#define SEARCHBAR_HPP

#include <QLineEdit>

class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

signals:
    void escapePressed();
    void focused();
    void blurred();
public:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    bool selectall_flag;
};

#endif // SEARCHBAR_HPP
