#ifndef RENDERHELPERS_HPP
#define RENDERHELPERS_HPP

#include <QByteArray>
#include <QTextCursor>
#include <QTextDocument>

namespace renderhelpers
{
    void renderEscapeCodes(const QByteArray &input, QTextCharFormat& format, const QTextCharFormat& defaultFormat, QTextCursor& cursor);

    /**
     * Replaces single and double quotes (', ") with one of the four
     * typographer's quotes, a.k.a curly quotes, e.g: ‘this’ and “this”
     */
    QByteArray replace_quotes(QByteArray &line);

    void setPageMargins(QTextDocument *doc, int mh, int mv);
}

#endif
