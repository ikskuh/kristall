#include "documentstyle.hpp"
#include <cassert>
#include <QDebug>
#include <QString>
#include <QStringList>

#include <QCryptographicHash>
#include <QDebug>

#include <array>
#include <cmath>

static QString encodeCssFont (const QFont& refFont)
{
    //-----------------------------------------------------------------------
    // This function assembles a CSS Font specification string from
    // a QFont. This supports most of the QFont attributes settable in
    // the Qt 4.8 and Qt 5.3 QFontDialog.
    //
    // (1) Font Family
    // (2) Font Weight (just bold or not)
    // (3) Font Style (possibly Italic or Oblique)
    // (4) Font Size (in either pixels or points)
    // (5) Decorations (possibly Underline or Strikeout)
    //
    // Not supported: Writing System (e.g. Latin).
    //
    // See the corresponding decode function, below.
    // QFont decodeCssFontString (const QString cssFontStr)
    //-----------------------------------------------------------------------

    QStringList fields; // CSS font attribute fields

    // ***************************************************
    // *** (1) Font Family: Primary plus Substitutes ***
    // ***************************************************

    const QString family = refFont.family();

    // NOTE [9-2014, Qt 4.8.6]: This isn't what I thought it was. It
    // does not return a list of "fallback" font faces (e.g. Georgia,
    // Serif for "Times New Roman"). In my testing, this is always
    // returning an empty list.
    //
    QStringList famSubs = QFont::substitutes (family);

    if (!famSubs.contains (family))
        famSubs.prepend (family);

    static const QChar DBL_QUOT ('"');
    const int famCnt = famSubs.count();
    QStringList famList;
    for (int inx = 0; inx < famCnt; ++inx)
    {
        // Place double quotes around family names having space characters,
        // but only if double quotes are not already there.
        //
        const QString fam = famSubs [inx];
        if (fam.contains (' ') && !fam.startsWith (DBL_QUOT))
            famList << (DBL_QUOT + fam + DBL_QUOT);
        else
            famList << fam;
    }

    const QString famStr = QString ("font-family: ") + famList.join (", ");
    fields << famStr;

    // **************************************
    // *** (2) Font Weight: Bold or Not ***
    // **************************************

    const bool bold = refFont.bold();
    if (bold)
        fields << "font-weight: bold";

    // ****************************************************
    // *** (3) Font Style: possibly Italic or Oblique ***
    // ****************************************************

    const QFont::Style style = refFont.style();
    switch (style)
    {
    case QFont::StyleNormal: break;
    case QFont::StyleItalic: fields << "font-style: italic"; break;
    case QFont::StyleOblique: fields << "font-style: oblique"; break;
    }

    // ************************************************
    // *** (4) Font Size: either Pixels or Points ***
    // ************************************************

    const double sizeInPoints = refFont.pointSizeF(); // <= 0 if not defined.
    const int sizeInPixels = refFont.pixelSize(); // <= 0 if not defined.
    if (sizeInPoints > 0.0)
        fields << QString ("font-size: %1pt") .arg (sizeInPoints);
    else if (sizeInPixels > 0)
        fields << QString ("font-size: %1px") .arg (sizeInPixels);

    // ***********************************************
    // *** (5) Decorations: Underline, Strikeout ***
    // ***********************************************

    const bool underline = refFont.underline();
    const bool strikeOut = refFont.strikeOut();

    if (underline && strikeOut)
        fields << "text-decoration: underline line-through";
    else if (underline)
        fields << "text-decoration: underline";
    else if (strikeOut)
        fields << "text-decoration: line-through";

    const QString cssFontStr = fields.join ("; ");
    return cssFontStr;
}

DocumentStyle::DocumentStyle() : theme(Fixed),
                             standard_font(),
                             h1_font(),
                             h2_font(),
                             h3_font(),
                             preformatted_font(),
                             background_color("#edefff"),
                             standard_color(0x00, 0x00, 0x00),
                             preformatted_color(0x00, 0x00, 0x00),
                             h1_color("#022f90"),
                             h2_color("#022f90"),
                             h3_color("#022f90"),
                             blockquote_color("#FFFFFF"),
                             internal_link_color("#0e8fff"),
                             external_link_color("#0e8fff"),
                             cross_scheme_link_color("#0960a7"),
                             internal_link_prefix("→ "),
                             external_link_prefix("⇒ "),
                             margin(55.0)
{
    preformatted_font.setFamily("monospace");
    preformatted_font.setPointSizeF(10.0);

    standard_font.setFamily("sans");
    standard_font.setPointSizeF(10.0);

    h1_font.setFamily("sans");
    h1_font.setBold(true);
    h1_font.setPointSizeF(20.0);

    h2_font.setFamily("sans");
    h2_font.setBold(true);
    h2_font.setPointSizeF(15.0);

    h3_font.setFamily("sans");
    h3_font.setBold(true);
    h3_font.setPointSizeF(12.0);
}

bool DocumentStyle::save(QSettings &settings) const
{
    settings.setValue("standard_font", standard_font.toString());
    settings.setValue("h1_font", h1_font.toString());
    settings.setValue("h2_font", h2_font.toString());
    settings.setValue("h3_font", h3_font.toString());
    settings.setValue("preformatted_font", preformatted_font.toString());

    settings.setValue("background_color", background_color.name());
    settings.setValue("standard_color", standard_color.name());
    settings.setValue("preformatted_color", preformatted_color.name());
    settings.setValue("blockquote_color", blockquote_color.name());
    settings.setValue("h1_color", h1_color.name());
    settings.setValue("h2_color", h2_color.name());
    settings.setValue("h3_color", h3_color.name());
    settings.setValue("internal_link_color", internal_link_color.name());
    settings.setValue("external_link_color", external_link_color.name());
    settings.setValue("cross_scheme_link_color", cross_scheme_link_color.name());

    settings.setValue("internal_link_prefix", internal_link_prefix);
    settings.setValue("external_link_prefix", external_link_prefix);

    settings.setValue("margins", margin);
    settings.setValue("theme", int(theme));
    return true;
}

bool DocumentStyle::load(QSettings &settings)
{
    if(settings.contains("standard_color"))
    {
        standard_font.fromString(settings.value("standard_font").toString());
        h1_font.fromString(settings.value("h1_font").toString());
        h2_font.fromString(settings.value("h2_font").toString());
        h3_font.fromString(settings.value("h3_font").toString());
        preformatted_font.fromString(settings.value("preformatted_font").toString());

        background_color = QColor(settings.value("background_color").toString());
        standard_color = QColor(settings.value("standard_color").toString());
        preformatted_color = QColor(settings.value("preformatted_color").toString());
        blockquote_color = QColor(settings.value("blockquote_color").toString());
        h1_color = QColor(settings.value("h1_color").toString());
        h2_color = QColor(settings.value("h2_color").toString());
        h3_color = QColor(settings.value("h3_color").toString());
        internal_link_color = QColor(settings.value("internal_link_color").toString());
        external_link_color = QColor(settings.value("external_link_color").toString());
        cross_scheme_link_color = QColor(settings.value("cross_scheme_link_color").toString());

        internal_link_prefix = settings.value("internal_link_prefix").toString();
        external_link_prefix = settings.value("external_link_prefix").toString();

        margin = settings.value("margins").toDouble();
        theme = Theme(settings.value("theme").toInt());
    }
    return true;
}

DocumentStyle DocumentStyle::derive(const QUrl &url) const
{
    if (this->theme == Fixed)
        return *this;

    QByteArray hash = QCryptographicHash::hash(url.host().toUtf8(), QCryptographicHash::Md5);

    std::array<uint8_t, 16> items;
    assert(items.size() == hash.size());
    memcpy(items.data(), hash.data(), items.size());

    float hue = (items[0] + items[1]) / 510.0;
    float saturation = items[2] / 255.0;

    double tmp;
    DocumentStyle themed = *this;
    switch (this->theme)
    {
    case AutoDarkTheme:
    {
        themed.background_color = QColor::fromHslF(hue, saturation, 0.25f);
        themed.standard_color = QColor{0xFF, 0xFF, 0xFF};

        themed.h1_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.75);
        themed.h2_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.75);
        themed.h3_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.75);

        themed.external_link_color = QColor::fromHslF(std::modf(hue + 0.25, &tmp), 1.0, 0.75);
        themed.internal_link_color = themed.external_link_color.lighter(110);
        themed.cross_scheme_link_color = themed.external_link_color.darker(110);

        themed.blockquote_color = themed.background_color.lighter(130);

        break;
    }

    case AutoLightTheme:
    {
        themed.background_color = QColor::fromHslF(hue, items[2] / 255.0, 0.85);
        themed.standard_color = QColor{0x00, 0x00, 0x00};

        themed.h1_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.25);
        themed.h2_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.25);
        themed.h3_color = QColor::fromHslF(std::modf(hue + 0.5, &tmp), 1.0 - saturation, 0.25);

        themed.external_link_color = QColor::fromHslF(std::modf(hue + 0.25, &tmp), 1.0, 0.25);
        themed.internal_link_color = themed.external_link_color.darker(110);
        themed.cross_scheme_link_color = themed.external_link_color.lighter(110);

        themed.blockquote_color = themed.background_color.darker(120);

        break;
    }

    case Fixed:
        assert(false);
    }

    // Same for all themes
    themed.preformatted_color = themed.standard_color;

    return themed;
}

QString DocumentStyle::toStyleSheet() const
{
    QString css;

    css += QString("p   { color: %2; %1 }\n").arg(encodeCssFont (standard_font)).arg(standard_color.name());
    css += QString("a   { color: %2; %1 }\n").arg(encodeCssFont (standard_font)).arg(external_link_color.name());
    css += QString("pre { color: %2; %1 }\n").arg(encodeCssFont (preformatted_font)).arg(preformatted_color.name());
    css += QString("h1  { color: %2; %1 }\n").arg(encodeCssFont (h1_font)).arg(h1_color.name());
    css += QString("h2  { color: %2; %1 }\n").arg(encodeCssFont (h2_font)).arg(h2_color.name());
    css += QString("h3  { color: %2; %1 }\n").arg(encodeCssFont (h3_font)).arg(h3_color.name());

    // qDebug() << "CSS → " << css;
    return css;
}
