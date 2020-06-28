#ifndef KRISTALLTEXTBROWSER_HPP
#define KRISTALLTEXTBROWSER_HPP

#include <QTextBrowser>

class KristallTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    KristallTextBrowser(QWidget * parent);


    void mouseReleaseEvent(QMouseEvent * event) override;

signals:
    void anchorClicked(QUrl const &, bool open_in_new_tab);

private: // slots
    void on_anchorClicked(QUrl const & url);

private:
    bool signal_new_tab = false;
};

#endif // KRISTALLTEXTBROWSER_HPP
