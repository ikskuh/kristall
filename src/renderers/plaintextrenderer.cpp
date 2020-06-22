#include "plaintextrenderer.hpp"

#include <QTextImageFormat>
#include <QTextCursor>

std::unique_ptr<QTextDocument> PlainTextRenderer::render(const QByteArray &input, const DocumentStyle &style)
{
    QTextCharFormat standard;
    standard.setFont(style.preformatted_font);
    standard.setForeground(style.preformatted_color);

    std::unique_ptr<QTextDocument> result = std::make_unique<QTextDocument>();
    result->setDocumentMargin(style.margin);

    QTextCursor cursor { result.get() };
    cursor.insertText(QString::fromUtf8(input), standard);

    return result;
}
