#include "geminirenderer.hpp"

#include <QTextList>
#include <QTextBlock>
#include <QList>
#include <QStringList>
#include <QDebug>

#include "kristall.hpp"

#include "textstyleinstance.hpp"

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
    TextStyleInstance text_style { themed_style };

    std::unique_ptr<GeminiDocument> result = std::make_unique<GeminiDocument>();
    result->setDocumentMargin(themed_style.margin);
    result->background_color = themed_style.background_color;
    result->setIndentWidth(20);

    bool emit_fancy_text = global_options.enable_text_decoration;

    QTextCursor cursor{result.get()};

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
                cursor.setBlockFormat(text_style.standard_format);
                verbatim = false;
            }
            else
            {
                cursor.setBlockFormat(text_style.preformatted_format);
                cursor.setCharFormat(text_style.preformatted);
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

                cursor.insertText(item, text_style.standard);
                continue;
            }
            else
            {
                if (current_list != nullptr)
                {
                    cursor.insertBlock();
                    cursor.setBlockFormat(text_style.standard_format);
                }
                current_list = nullptr;
            }

            if(line.startsWith(">"))
            {
                if(not blockquote ) {
                    // cursor.insertBlock();
                }
                blockquote  = true;

                cursor.setBlockFormat(text_style.block_quote_format);
                cursor.insertText(trim_whitespace(line.mid(1)) + "\n", text_style.standard);

                continue;
            }
            else
            {
                if(blockquote) {
                    cursor.setBlockFormat(text_style.standard_format);
                }
                blockquote  = false;
            }

            if (line.startsWith("###"))
            {
                auto heading = trim_whitespace(line.mid(3));

                auto id = unique_anchor_name();
                auto fmt = text_style.standard_h3;
                fmt.setAnchor(true);
                fmt.setAnchorNames(QStringList { id });

                cursor.insertText(heading + "\n", fmt);
                outline.appendH3(heading, id);
            }
            else if (line.startsWith("##"))
            {
                auto heading = trim_whitespace(line.mid(2));

                auto id = unique_anchor_name();
                auto fmt = text_style.standard_h2;
                fmt.setAnchor(true);
                fmt.setAnchorNames(QStringList { id });

                cursor.insertText(heading + "\n", fmt);
                outline.appendH2(heading, id);
            }
            else if (line.startsWith("#"))
            {
                auto heading = trim_whitespace(line.mid(1));

                auto id = unique_anchor_name();
                auto fmt = text_style.standard_h1;
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

                auto fmt = text_style.standard_link;

                QString prefix;
                if (absolute_url.host() == root_url.host())
                {
                    prefix = themed_style.internal_link_prefix;
                    fmt = text_style.standard_link;
                }
                else
                {
                    prefix = themed_style.external_link_prefix;
                    fmt = text_style.external_link;
                }

                QString suffix = "";
                if (absolute_url.scheme() != root_url.scheme())
                {
                    if(absolute_url.scheme() != "kristall+ctrl") {
                        suffix = " [" + absolute_url.scheme().toUpper() + "]";
                        fmt = text_style.cross_protocol_link;
                    }
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

                    QTextCharFormat fmt = text_style.standard;

                    QByteArray buffer;

                    auto flush = [&]() {
                        if(buffer.size() > 0) {
                            cursor.insertText(QString::fromUtf8(buffer), fmt);
                            buffer.resize(0);
                        }
                    };

                    for(int i = 0; i < line.length(); i += 1)
                    {
                        char c = line.at(i);
                        if(c == ' ') {
                            flush();
                            fmt = text_style.standard;
                            buffer.append(' ');
                            rendering_bold = false;
                            rendering_underlined = false;
                        }
                        else if(c == '*') {
                            if(rendering_bold) {
                                buffer.append('*');
                            }
                            flush();
                            rendering_bold = not rendering_bold;
                            auto f = fmt.font();
                            f.setBold(rendering_bold);
                            fmt.setFont(f);
                            if(rendering_bold) {
                                buffer.append('*');
                            }
                        }
                        else if(c == '_') {
                            if(rendering_underlined) {
                                buffer.append(' ');
                            }
                            flush();
                            rendering_underlined = not rendering_underlined;
                            auto f = fmt.font();
                            fmt.setUnderlineStyle(rendering_underlined ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline);
                            if(rendering_underlined) {
                                buffer.append(' ');
                            }
                        }
                        else {
                            buffer.append(c);
                        }
                    }

                    flush();

                    cursor.insertText("\n", text_style.standard);
                }
                else {
                    cursor.insertText(line + "\n", text_style.standard);
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
