#ifndef RENDERHELPERS_HPP
#define RENDERHELPERS_HPP

#include <QByteArray>
#include <QTextCursor>
#include <QTextDocument>

void RenderEscapeCodes(const QByteArray &input, const QTextCharFormat& format, QTextCursor& cursor);

namespace renderhelpers
{
    void setPageMargins(QTextDocument *doc, int mh, int mv);
}

#endif
