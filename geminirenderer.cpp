#include "geminirenderer.hpp"

#include <QTextList>
#include <QCryptographicHash>
#include <QTextBlock>
#include <QDebug>
#include <cmath>

static QByteArray trim_whitespace(QByteArray items)
{
    int start = 0;
    while (start < items.size() and isspace(items.at(start)))
    {
        start += 1;
    }
    int end = items.size() - 1;
    while (end > 0 and isspace(items.at(end)))
    {
        end -= 1;
    }
    return items.mid(start, end - start + 1);
}

GeminiStyle::GeminiStyle() : theme(Fixed),
                             standard_font(),
                             h1_font(),
                             h2_font(),
                             h3_font(),
                             preformatted_font(),
                             background_color(0xFF, 0xFF, 0xFF),
                             standard_color(0x00, 0x00, 0x00),
                             preformatted_color(0x00, 0x00, 0x00),
                             h1_color(0xFF, 0x00, 0x00),
                             h2_color(0x00, 0x80, 0x00),
                             h3_color(0x80, 0xFF, 0x00),
                             internal_link_color(0x00, 0x80, 0x0FF),
                             external_link_color(0x00, 0x00, 0xFF),
                             cross_scheme_link_color(0x80, 0x00, 0xFF),
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

bool GeminiStyle::save(QSettings &settings) const
{
    settings.beginGroup("Theme");

    settings.setValue("standard_font", standard_font.toString());
    settings.setValue("h1_font", h1_font.toString());
    settings.setValue("h2_font", h2_font.toString());
    settings.setValue("h3_font", h3_font.toString());
    settings.setValue("preformatted_font", preformatted_font.toString());

    settings.setValue("background_color", background_color.name());
    settings.setValue("standard_color", standard_color.name());
    settings.setValue("preformatted_color", preformatted_color.name());
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

    settings.endGroup();
    return true;
}

bool GeminiStyle::load(QSettings &settings)
{
    settings.beginGroup("Theme");

    standard_font.fromString(settings.value("standard_font").toString());
    h1_font.fromString(settings.value("h1_font").toString());
    h2_font.fromString(settings.value("h2_font").toString());
    h3_font.fromString(settings.value("h3_font").toString());
    preformatted_font.fromString(settings.value("preformatted_font").toString());

    background_color = QColor(settings.value("background_color").toString());
    standard_color = QColor(settings.value("standard_color").toString());
    preformatted_color = QColor(settings.value("preformatted_color").toString());
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

    settings.endGroup();
    return true;
}

GeminiStyle GeminiStyle::derive(const QUrl &url) const
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
    GeminiStyle themed = *this;
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

        break;
    }

    case Fixed:
        assert(false);
    }

    // Same for all themes
    themed.preformatted_color = themed.standard_color;

    return themed;
}

GeminiRenderer::GeminiRenderer(GeminiStyle const &_style) : style(_style)
{
}

std::unique_ptr<GeminiDocument> GeminiRenderer::render(const QByteArray &input, QUrl const &root_url, DocumentOutlineModel &outline)
{
    auto themed_style = style.derive(root_url);

    QTextCharFormat preformatted;
    preformatted.setFont(themed_style.preformatted_font);
    preformatted.setForeground(themed_style.preformatted_color);

    QTextCharFormat standard;
    standard.setFont(themed_style.standard_font);
    standard.setForeground(themed_style.standard_color);

    QTextCharFormat standard_link;
    standard_link.setFont(themed_style.standard_font);
    standard_link.setForeground(QBrush(themed_style.internal_link_color));

    QTextCharFormat external_link;
    external_link.setFont(themed_style.standard_font);
    external_link.setForeground(QBrush(themed_style.external_link_color));

    QTextCharFormat cross_protocol_link;
    cross_protocol_link.setFont(themed_style.standard_font);
    cross_protocol_link.setForeground(QBrush(themed_style.cross_scheme_link_color));

    QTextCharFormat standard_h1;
    standard_h1.setFont(themed_style.h1_font);
    standard_h1.setForeground(QBrush(themed_style.h1_color));

    QTextCharFormat standard_h2;
    standard_h2.setFont(themed_style.h2_font);
    standard_h2.setForeground(QBrush(themed_style.h2_color));

    QTextCharFormat standard_h3;
    standard_h3.setFont(themed_style.h3_font);
    standard_h3.setForeground(QBrush(themed_style.h3_color));

    std::unique_ptr<GeminiDocument> result = std::make_unique<GeminiDocument>();
    result->setDocumentMargin(themed_style.margin);
    result->background_color = themed_style.background_color;

    QTextCursor cursor{result.get()};

    QTextBlockFormat non_list_format = cursor.blockFormat();

    bool verbatim = false;
    QTextList *current_list = nullptr;

    outline.beginBuild();

    QList<QByteArray> lines = input.split('\n');
    for (auto const &line : lines)
    {
        if (verbatim)
        {
            if (line.startsWith("```"))
            {
                verbatim = false;
            }
            else
            {
                cursor.setCharFormat(preformatted);
                cursor.insertText(line + "\n");
            }
        }
        else
        {
            if (line.startsWith("*"))
            {
                if (current_list == nullptr)
                {
                    cursor.deletePreviousChar();
                    current_list = cursor.insertList(QTextListFormat::ListDisc);
                }
                else
                {
                    cursor.insertBlock();
                }

                QString item = trim_whitespace(line.mid(1));

                cursor.insertText(item, standard);
                continue;
            }
            else
            {
                if (current_list != nullptr)
                {
                    cursor.insertBlock();
                    cursor.setBlockFormat(non_list_format);
                }
                current_list = nullptr;
            }

            if (line.startsWith("###"))
            {
                auto heading = trim_whitespace(line.mid(3));

                cursor.insertText(heading + "\n", standard_h3);
                outline.appendH3(heading);
            }
            else if (line.startsWith("##"))
            {
                auto heading = trim_whitespace(line.mid(2));

                cursor.insertText(heading + "\n", standard_h2);
                outline.appendH2(heading);
            }
            else if (line.startsWith("#"))
            {
                auto heading = trim_whitespace(line.mid(1));

                cursor.insertText(heading + "\n", standard_h1);
                outline.appendH1(heading);
            }
            else if (line.startsWith("=>"))
            {
                auto const part = line.mid(2).trimmed();

                QByteArray link, title;

                int index = -1;
                for (int i = 0; i < part.size(); i++)
                {
                    if (isspace(part[i]))
                    {
                        index = i;
                        break;
                    }
                }

                if (index > 0)
                {
                    link = trim_whitespace(part.mid(0, index));
                    title = trim_whitespace(part.mid(index + 1));
                }
                else
                {
                    link = trim_whitespace(part);
                    title = trim_whitespace(part);
                }

                auto local_url = QUrl(link);

                auto absolute_url = root_url.resolved(QUrl(link));

                // qDebug() << link << title;

                auto fmt = standard_link;

                QString prefix;
                if (absolute_url.host() == root_url.host())
                {
                    prefix = themed_style.internal_link_prefix;
                    fmt = standard_link;
                }
                else
                {
                    prefix = themed_style.external_link_prefix;
                    fmt = external_link;
                }

                QString suffix = "";
                if (absolute_url.scheme() != root_url.scheme())
                {
                    suffix = " [" + absolute_url.scheme().toUpper() + "]";
                    fmt = cross_protocol_link;
                }

                fmt.setAnchor(true);
                fmt.setAnchorHref(absolute_url.toString());
                cursor.insertText(prefix + title + suffix + "\n", fmt);
            }
            else if (line.startsWith("```"))
            {
                verbatim = true;
            }
            else
            {
                cursor.insertText(line + "\n", standard);
            }
        }
    }

    outline.endBuild();
    return result;
}

GeminiDocument::GeminiDocument(QObject *parent) : QTextDocument(parent),
                                                  background_color(0x00, 0x00, 0x00)
{
}

GeminiDocument::~GeminiDocument()
{
}
