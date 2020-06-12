#include "elidelabel.hpp"

#include <QPainter>

ElideLabel::ElideLabel(QWidget* parent)
    : QLabel(parent)
{
    m_elideMode = Qt::ElideNone;
}

ElideLabel::ElideLabel(const QString &text, QWidget* parent) : QLabel(text, parent)
{
    m_elideMode = Qt::ElideNone;
}

ElideLabel::~ElideLabel()
{
}

void ElideLabel::setElideMode(Qt::TextElideMode mode)
{
    m_elideMode = mode;
}

Qt::TextElideMode ElideLabel::elideMode() const
{
    return m_elideMode;
}

void ElideLabel::paintEvent(QPaintEvent * event)
{
    if (m_elideMode == Qt::ElideNone)
    {
        QLabel::paintEvent(event);
    }
    else
    {
        QFrame::paintEvent(event);
        QPainter painter(this);
        QRect r = contentsRect();
        painter.drawText(r, alignment(), fontMetrics().elidedText(text(), m_elideMode, r.width()));
    }
}

QSize ElideLabel::minimumSizeHint() const
{
    if (m_elideMode != Qt::ElideNone)
    {
        const QFontMetrics& fm = fontMetrics();
        QSize size(fm.width("..."), fm.height());
        return size;
    }

    return QLabel::minimumSizeHint();
}
