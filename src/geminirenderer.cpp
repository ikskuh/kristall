#include "geminirenderer.hpp"

#include <QTextList>
#include <QTextBlock>
#include <QList>
#include <QStringList>
#include <QDebug>

#include "kristall.hpp"

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

std::unique_ptr<GeminiDocument> GeminiRenderer::render(
        const QByteArray &input,
        QUrl const &root_url,
        DocumentStyle const & themed_style,
        DocumentOutlineModel &outline)
{
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
    result->setIndentWidth(20);

    bool emit_fancy_text = global_options.enable_text_decoration;

    QTextCursor cursor{result.get()};

    QTextBlockFormat standard_format = cursor.blockFormat();

    QTextBlockFormat preformatted_format = standard_format;
    preformatted_format.setNonBreakableLines(true);

    QTextBlockFormat block_quote_format = standard_format;
    block_quote_format.setIndent(1);
    block_quote_format.setBackground(themed_style.blockquote_color);


    bool verbatim = false;
    QTextList *current_list = nullptr;
    bool blockquote = false;

    outline.beginBuild();

    int anchor_id = 0;

    auto unique_anchor_name = [&]() -> QString {
        return QString("auto-title-%1").arg(++anchor_id);
    };

    QList<QByteArray> lines = input.split('\n');
    for (auto const &line : lines)
    {
        if (verbatim)
        {
            if (line.startsWith("```"))
            {
                cursor.setBlockFormat(standard_format);
                verbatim = false;
            }
            else
            {
                cursor.setBlockFormat(preformatted_format);
                cursor.setCharFormat(preformatted);
                cursor.insertText(line + "\n");
            }
        }
        else
        {
            if (line.startsWith("* "))
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
                    cursor.setBlockFormat(standard_format);
                }
                current_list = nullptr;
            }

            if(line.startsWith(">"))
            {
                if(not blockquote ) {
                    // cursor.insertBlock();
                }
                blockquote  = true;

                cursor.setBlockFormat(block_quote_format);
                cursor.insertText(trim_whitespace(line.mid(1)) + "\n", standard);

                continue;
            }
            else
            {
                if(blockquote) {
                    cursor.setBlockFormat(standard_format);
                }
                blockquote  = false;
            }

            if (line.startsWith("###"))
            {
                auto heading = trim_whitespace(line.mid(3));

                auto id = unique_anchor_name();
                auto fmt = standard_h3;
                fmt.setAnchor(true);
                fmt.setAnchorNames(QStringList { id });

                cursor.insertText(heading + "\n", fmt);
                outline.appendH3(heading, id);
            }
            else if (line.startsWith("##"))
            {
                auto heading = trim_whitespace(line.mid(2));

                auto id = unique_anchor_name();
                auto fmt = standard_h2;
                fmt.setAnchor(true);
                fmt.setAnchorNames(QStringList { id });

                cursor.insertText(heading + "\n", fmt);
                outline.appendH2(heading, id);
            }
            else if (line.startsWith("#"))
            {
                auto heading = trim_whitespace(line.mid(1));

                auto id = unique_anchor_name();
                auto fmt = standard_h1;
                fmt.setAnchor(true);
                fmt.setAnchorNames(QStringList { id });

                cursor.insertText(heading + "\n", fmt);
                outline.appendH1(heading, id);
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
                if(emit_fancy_text)
                {
                    // TODO: Fix UTF-8 encoding here… Don't emit single characters but always spans!

                    bool rendering_bold = false;
                    bool rendering_underlined = false;

                    QTextCharFormat fmt = standard;

                    for(int i = 0; i < line.length(); i += 1)
                    {
                        char c = line.at(i);
                        if(c == ' ') {
                            fmt = standard;
                            cursor.insertText(" ");
                            rendering_bold = false;
                            rendering_underlined = false;
                        }
                        else if(c == '*') {
                            if(rendering_bold)
                                cursor.insertText("*", fmt);
                            rendering_bold = not rendering_bold;
                            auto f = fmt.font();
                            f.setBold(rendering_bold);
                            fmt.setFont(f);
                            if(rendering_bold)
                                cursor.insertText("*", fmt);
                        }
                        else if(c == '_') {
                            if(rendering_underlined)
                                cursor.insertText(" ", fmt);
                            rendering_underlined = not rendering_underlined;
                            auto f = fmt.font();
                            fmt.setUnderlineStyle(rendering_underlined ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline);
                            if(rendering_underlined)
                                cursor.insertText(" ", fmt);
                        }
                        else {
                            cursor.insertText(QString::fromUtf8(&c, 1), fmt);
                        }
                    }

                    cursor.insertText("\n", standard);
                }
                else {
                    cursor.insertText(line + "\n", standard);
                }
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
