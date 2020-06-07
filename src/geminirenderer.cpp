#include "geminirenderer.hpp"

#include <QTextList>
#include <QTextBlock>
#include <QList>
#include <QStringList>

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
    QTextOption no_wrap;
    no_wrap.setWrapMode(QTextOption::NoWrap);

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


    result->setDefaultTextOption(no_wrap);

    QTextCursor cursor{result.get()};

    QTextBlockFormat non_list_format = cursor.blockFormat();

    bool verbatim = false;
    QTextList *current_list = nullptr;

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
                verbatim = false;
            }
            else
            {
                cursor.block().layout()->setTextOption(no_wrap);
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
