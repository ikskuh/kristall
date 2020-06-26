#ifndef KRISTALLTEXTBROWSER_HPP
#define KRISTALLTEXTBROWSER_HPP

#include <QTextBrowser>

class KristallTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    KristallTextBrowser(QWidget * parent);


    void mouseReleaseEvent(QMouseEvent * event) override;


public:
    Qt::MouseButton last_button;
};

#endif // KRISTALLTEXTBROWSER_HPP
