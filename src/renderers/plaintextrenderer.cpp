
#include "plaintextrenderer.hpp"
#include "renderhelpers.hpp"
#include <QByteArray>
#include <QTextImageFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <memory>

std::unique_ptr<QTextDocument> PlainTextRenderer::render(const QByteArray &input, const DocumentStyle &style)
{
    QTextCharFormat standard;
    standard.setFont(style.preformatted_font);
    standard.setForeground(style.preformatted_color);

    std::unique_ptr<QTextDocument> result = std::make_unique<QTextDocument>();
    renderhelpers::setPageMargins(result.get(), style.margin_h, style.margin_v);

    QTextCursor cursor { result.get() };
    QTextCharFormat text_fmt = standard;
    renderhelpers::renderEscapeCodes(input, text_fmt, standard, cursor);

    return result;
}
