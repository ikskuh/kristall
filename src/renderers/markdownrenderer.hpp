#ifndef MARKDOWNRENDERER_HPP
#define MARKDOWNRENDERER_HPP


#include "documentstyle.hpp"
#include "documentoutlinemodel.hpp"

#include <memory>
#include <QTextDocument>

struct MarkdownRenderer
{
    MarkdownRenderer() = delete;


    //! Renders the given byte sequence into a GeminiDocument.
    //! @param input    The utf8 encoded input string
    //! @param root_url The url that is used to resolve relative links
    //! @param style    The style which is used to render the document
    //! @param outline  The extracted outline from the document
    static std::unique_ptr<QTextDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentStyle const & style,
        DocumentOutlineModel & outline
    );
};

#endif // MARKDOWNRENDERER_HPP
