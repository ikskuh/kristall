#ifndef HTMLRENDERER_HPP
#define HTMLRENDERER_HPP

#include <memory>
#include <QTextDocument>
#include <QColor>
#include <QSettings>

#include "documentoutlinemodel.hpp"

#include "documentstyle.hpp"

struct HtmlRenderer
{
    HtmlRenderer() = delete;

    //! Renders the given byte sequence into a GeminiDocument.
    //! @param input    The utf8 encoded input string
    //! @param root_url The url that is used to resolve relative links
    //! @param style    The style which is used to render the document
    //! @param outline  The extracted outline from the document
    static std::unique_ptr<QTextDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentStyle const & style,
        DocumentOutlineModel & outline,
        QString & page_title
    );
};

#endif // HTMLRENDERER_HPP
