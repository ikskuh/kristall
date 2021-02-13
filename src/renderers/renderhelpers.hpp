#ifndef RENDERHELPERS_HPP
#define RENDERHELPERS_HPP

#include <QByteArray>
#include <QTextCursor>
#include <QTextDocument>

namespace renderhelpers
{
    void renderEscapeCodes(const QByteArray &input, const QTextCharFormat& format, QTextCursor& cursor);

    void setPageMargins(QTextDocument *doc, int mh, int mv);
}

#endif
