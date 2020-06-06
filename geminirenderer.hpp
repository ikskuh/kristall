#ifndef GEMINIRENDERER_HPP
#define GEMINIRENDERER_HPP

#include <QTextDocument>
#include <QColor>

#include "documentoutlinemodel.hpp"

struct GeminiStyle
{
    enum Theme {
        Fixed = 0,
        AutoDarkTheme = 1,
        AutoLightTheme = 2
    };

    GeminiStyle();

    Theme theme;

    QFont standard_font;
    QFont h1_font;
    QFont h2_font;
    QFont h3_font;
    QFont preformatted_font;

    QColor background_color;
    QColor standard_color;
    QColor preformatted_color;
    QColor h1_color;
    QColor h2_color;
    QColor h3_color;

    QColor internal_link_color;
    QColor external_link_color;
    QColor cross_scheme_link_color;

    QString internal_link_prefix;
    QString external_link_prefix;

    double margin;

    //! Create a new style with auto-generated colors for the given
    //! url. The colors are based on the host name
    GeminiStyle derive(QUrl const & url) const;
};

class GeminiDocument :
        public QTextDocument
{
    Q_OBJECT
public:
    explicit GeminiDocument(QObject * parent = nullptr);
    ~GeminiDocument() override;

    QColor background_color;
};

class GeminiRenderer
{
    GeminiStyle style;
public:
    GeminiRenderer(GeminiStyle const & style = GeminiStyle{});

    std::unique_ptr<GeminiDocument> render(
        QByteArray const & input,
        QUrl const & root_url,
        DocumentOutlineModel & outline
    );
};

#endif // GEMINIRENDERER_HPP
