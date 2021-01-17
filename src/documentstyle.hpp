#ifndef DOCUMENTSTYLE_HPP
#define DOCUMENTSTYLE_HPP

#include <QUrl>
#include <QFont>
#include <QColor>
#include <QSettings>

struct DocumentStyle
{
    enum Theme {
        Fixed = 0,
        AutoDarkTheme = 1,
        AutoLightTheme = 2
    };

    DocumentStyle(bool do_init = true);

    void initialiseDefaultFonts();

    //! Calculates a filtered/legal file name with all non-allowed chars escaped
    static QString createFileNameFromName(QString const & src, int index);

    Theme theme;

    //! This is only a runtime value that is used to create
    //! unique font names in `derive()`.
    //! This is a terrible hack
    QString cookie;

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
    QColor blockquote_color;

    QColor internal_link_color;
    QColor external_link_color;
    QColor cross_scheme_link_color;

    QString internal_link_prefix;
    QString external_link_prefix;

    double margin;

    QStringList ansi_colors;

    bool save(QSettings & settings) const;
    bool load(QSettings & settings);

    //! Create a new style with auto-generated colors for the given
    //! url. The colors are based on the host name
    DocumentStyle derive(QUrl const & url) const;

    //! Converts this style into a CSS document for
    //! non-gemini rendered files.
    QString toStyleSheet() const;
};

#endif // DOCUMENTSTYLE_HPP
