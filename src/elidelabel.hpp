#ifndef ELIDELABEL_HPP
#define ELIDELABEL_HPP

#include <QLabel>

class ElideLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)

public:
    ElideLabel(QWidget* parent = 0);
    ElideLabel(const QString &text, QWidget* parent = 0);
    ~ElideLabel();

    void setElideMode(Qt::TextElideMode mode);
    Qt::TextElideMode elideMode() const;

    QSize minimumSizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent * event);

private:
    Qt::TextElideMode m_elideMode;
};

#endif // ELIDELABEL_HPP
