#ifndef KRISTALLTEXTBROWSER_HPP
#define KRISTALLTEXTBROWSER_HPP

#include <QTextBrowser>

class KristallTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    KristallTextBrowser(QWidget * parent);


    void mouseReleaseEvent(QMouseEvent * event) override;

    void mouseMoveEvent(QMouseEvent * event) override;

    void setDefaultCursor(QCursor const & shape);

signals:
    void anchorClicked(QUrl const &, bool open_in_new_tab);

private: // slots
    void on_anchorClicked(QUrl const & url);

private:
    bool signal_new_tab = false;
    QCursor wanted_cursor;
};

#endif // KRISTALLTEXTBROWSER_HPP
