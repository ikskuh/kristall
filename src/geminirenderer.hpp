#ifndef GEMINIRENDERER_HPP
#define GEMINIRENDERER_HPP

#include <QTextDocument>
#include <QColor>
#include <QSettings>

#include "documentoutlinemodel.hpp"

#include "documentstyle.hpp"

class GeminiDocument :
        public QTextDocument
{
    Q_OBJECT
public:
    explicit GeminiDocument(QObject * parent = nullptr);
    ~GeminiDocument() override;

    QColor background_color;
};

struct GeminiRenderer
{
    GeminiRenderer() = delete;

    //! Renders the given byte sequence into a GeminiDocument.
    //! @param input    The utf8 encoded input string
    //! @param root_url The url that is used to resolve relative links
    //! @param style    The style which is used to render the document
    //! @param outline  The extracted outline from the document
    static std::unique_ptr<GeminiDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentStyle const & style,
        DocumentOutlineModel & outline
    );
};

#endif // GEMINIRENDERER_HPP
