#ifndef BROWSERTABS_HPP
#define BROWSERTABS_HPP

#include <QTabBar>
#include <QPushButton>

class BrowserTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit BrowserTabBar(QWidget * parent);

    void mouseReleaseEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void tabLayoutChange() override;

signals:
    void on_newTabClicked();

private:
    void moveNewTabButton();

private:
    QPushButton *newTabBtn;
};

#endif // BROWSERTABS_HPP
