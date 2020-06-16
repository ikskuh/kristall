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
public:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // SEARCHBAR_HPP
