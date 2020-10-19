#ifndef RENDERHELPERS_HPP
#define RENDERHELPERS_HPP

#include <QByteArray>
#include <QTextCursor>

void RenderEscapeCodes(const QByteArray &input, const QTextCharFormat& format, QTextCursor& cursor);

#endif
