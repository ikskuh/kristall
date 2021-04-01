#ifndef GEMINIRENDERER_HPP
#define GEMINIRENDERER_HPP

#include <memory>
#include <QTextDocument>

#include "documentoutlinemodel.hpp"

#include "documentstyle.hpp"

struct GeminiRenderer
{
    GeminiRenderer() = delete;

    //! Renders the given byte sequence into a GeminiDocument.
    //! @param input    The utf8 encoded input string
    //! @param root_url The url that is used to resolve relative links
    //! @param style    The style which is used to render the document
    //! @param outline  The extracted outline from the document
    //! @param page_title The extracted page title
    static std::unique_ptr<QTextDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentStyle const & style,
        DocumentOutlineModel & outline,
        QString & page_title
    );
};

#endif // GEMINIRENDERER_HPP
