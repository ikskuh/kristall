#ifndef BROWSERTABS_HPP
#define BROWSERTABS_HPP

#include <QTabBar>

class BrowserTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit BrowserTabBar(QWidget * parent);

    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // BROWSERTABS_HPP
