#include "geminirenderer.hpp"
#include "renderhelpers.hpp"

#include <QTextList>
#include <QTextBlock>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QTextTable>
#include <QRegularExpression>

#include "kristall.hpp"

#include "textstyleinstance.hpp"

static QByteArray trim_whitespace(const QByteArray &items)
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

static QByteArray replace_quotes(QByteArray&);
static void insertText(QTextCursor&, const QByteArray&, const QTextCharFormat&);

std::unique_ptr<GeminiDocument> GeminiRenderer::render(
        const QByteArray &input,
        QUrl const &root_url,
        DocumentStyle const & themed_style,
        DocumentOutlineModel &outline,
        QString* const page_title)
{
    TextStyleInstance text_style { themed_style };

    std::unique_ptr<GeminiDocument> result = std::make_unique<GeminiDocument>();
    renderhelpers::setPageMargins(result.get(), themed_style.margin_h, themed_style.margin_v);
    result->setIndentWidth(themed_style.indent_size);

    QTextCursor cursor{result.get()};

    bool verbatim = false;
    QTextList *current_list = nullptr;
    bool blockquote = false;

    bool centre_first_h1 = themed_style.centre_h1;

    QTextCharFormat preformatted_fmt = text_style.preformatted;

    outline.beginBuild();

    int anchor_id = 0;

    auto unique_anchor_name = [&]() -> QString {
        return QString("auto-title-%1").arg(++anchor_id);
    };

    QList<QByteArray> lines = input.split('\n');
    for (auto &line : lines)
    {
        if (verbatim)
        {
            if (line.startsWith("```"))
            {
                // Set the last line of the preformatted block to have
                // standard line height.
                QTextBlockFormat fmt = text_style.preformatted_format;
                fmt.setLineHeight(themed_style.line_height_p,
                    QTextBlockFormat::LineDistanceHeight);
                cursor.movePosition(QTextCursor::PreviousBlock);
                cursor.setBlockFormat(fmt);

                cursor.movePosition(QTextCursor::NextBlock);
                cursor.setBlockFormat(text_style.standard_format);
                verbatim = false;
            }
            else
            {
                cursor.setBlockFormat(text_style.preformatted_format);
                renderhelpers::renderEscapeCodes(line, preformatted_fmt, text_style.preformatted, cursor);
                cursor.insertText("\n", text_style.preformatted);
            }

            continue;
        }

        // List item
        if (line.startsWith("* "))
        {
            if (current_list == nullptr)
            {
                cursor.deletePreviousChar();
                cursor.insertBlock();
                cursor.setBlockFormat(text_style.standard_format);
                current_list = cursor.createList(text_style.list_format);
            }
            else
            {
                cursor.insertBlock();
            }

            replace_quotes(line);
            insertText(cursor, trim_whitespace(line.mid(1)), text_style.standard);
            continue;
        }

        // End of list
        if (current_list != nullptr)
        {
            cursor.insertBlock();
            cursor.setBlockFormat(text_style.standard_format);
        }
        current_list = nullptr;

        // Block quote
        if(line.startsWith(">"))
        {
            if(!blockquote)
            {
                // Start blockquote
                QTextTable *table = cursor.insertTable(1, 1, text_style.blockquote_tableformat);
                cursor.setBlockFormat(text_style.blockquote_format);
                QTextTableCell cell = table->cellAt(0, 0);
                cell.setFormat(text_style.blockquote);
                blockquote = true;
            }

            replace_quotes(line);
            insertText(cursor, trim_whitespace(line.mid(1)), text_style.blockquote);
            cursor.insertText("\n", text_style.standard);
            continue;
        }

        // End of blockquote
        if (blockquote)
        {
            cursor.deletePreviousChar();
            cursor.movePosition(QTextCursor::NextBlock);
            cursor.setBlockFormat(text_style.standard_format);
        }
        blockquote = false;

        // Headings, etc.
        if (line.startsWith("###"))
        {
            auto heading = trim_whitespace(line.mid(3));

            auto id = unique_anchor_name();
            auto fmt = text_style.standard_h3;
            fmt.setAnchor(true);
            fmt.setAnchorNames(QStringList { id });

            outline.appendH3(heading, id);

            cursor.setBlockFormat(text_style.heading_format);
            insertText(cursor, replace_quotes(heading), fmt);
            cursor.insertText("\n", text_style.standard);
        }
        else if (line.startsWith("##"))
        {
            auto heading = trim_whitespace(line.mid(2));

            auto id = unique_anchor_name();
            auto fmt = text_style.standard_h2;
            fmt.setAnchor(true);
            fmt.setAnchorNames(QStringList { id });

            outline.appendH2(heading, id);

            cursor.setBlockFormat(text_style.heading_format);
            insertText(cursor, replace_quotes(heading), fmt);
            cursor.insertText("\n", text_style.standard);
        }
        else if (line.startsWith("#"))
        {
            auto heading = trim_whitespace(line.mid(1));

            auto id = unique_anchor_name();
            auto fmt = text_style.standard_h1;
            fmt.setAnchor(true);
            fmt.setAnchorNames(QStringList { id });

            outline.appendH1(heading, id);

            // Use first heading as the page's title.
            if (page_title != nullptr && page_title->isEmpty())
            {
                *page_title = heading;
            }

            // Centre the first heading. We can't use the above code block
            // for this because it doesn't get run on every re-render of the page
            if (centre_first_h1)
            {
                auto f = text_style.heading_format;
                f.setAlignment(Qt::AlignCenter);
                cursor.setBlockFormat(f);
                centre_first_h1 = false;
            }
            else
            {
                cursor.setBlockFormat(text_style.heading_format);
            }

            insertText(cursor, replace_quotes(heading), fmt);
            cursor.insertText("\n", text_style.standard);
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
            replace_quotes(title);

            auto local_url = QUrl(link);

            // Makes relative URLs with scheme provided (e.g gemini:///relative) work
            // From RFC 1630: "If the scheme parts are different, the whole absolute URI must be given"
            // therefor the schemes must be same for this to be allowed.
            if (local_url.scheme() == root_url.scheme() &&
                local_url.authority().isEmpty() &&
                local_url.scheme() != "about" &&
                local_url.scheme() != "file")
            {
                // qDebug() << "Adjusting local url: " << local_url;
                local_url = local_url.adjusted(QUrl::RemoveScheme | QUrl::RemoveAuthority);
            }
            auto absolute_url = root_url.resolved(local_url);

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
            cursor.setBlockFormat(text_style.link_format);
            insertText(cursor, (prefix + title + suffix).toUtf8(), fmt);
            cursor.insertText("\n", text_style.standard);
        }
        else if (line.startsWith("```"))
        {
            verbatim = true;
            preformatted_fmt = text_style.preformatted;
        }
        else
        {
            cursor.setBlockFormat(text_style.standard_format);

            replace_quotes(line);
            insertText(cursor, line, text_style.standard);
            cursor.insertText("\n", text_style.standard);
        }
    }

    outline.endBuild();
    return result;
}

GeminiDocument::GeminiDocument(QObject *parent) : QTextDocument(parent)
{
}

GeminiDocument::~GeminiDocument()
{
}

/*
 * This replaces single and double quotes (', ") with
 * one of the four typographer's quotes, a.k.a curly quotes,
 * e.g: ‘this’ and “this”
 */
static QByteArray replace_quotes(QByteArray &line)
{
    if (!kristall::globals().options.fancy_quotes)
        return line;

    int last_d = -1,
        last_s = -1;

    for (int i = 0; i < line.length(); ++i)
    {
        // Double quotes
        if (line[i] == '"')
        {
            if (last_d == -1)
            {
                last_d = i;
            }
            else
            {
                // Replace quote at first position:
                QByteArray first = QString("“").toUtf8();
                line.replace(last_d, 1, first);

                // Replace quote at second position:
                line.replace(i + first.size() - 1, 1, QString("”").toUtf8());

                last_d = -1;
            }
        }
        else if (line[i] == '\'')
        {
            if (last_s == -1)
            {
                // Skip if it looks like a contraction rather
                // than a quote.
                if (i > 0 && line[i - 1] != ' ')
                {
                    line.replace(i, 1, QString("’").toUtf8());
                    continue;
                }

                // For shortenings like 'till
                int len = line.length();
                if ((i + 1) < len && line[i + 1] != ' ')
                {
                    line.replace(i, 1, QString("‘").toUtf8());
                    continue;
                }

                last_s = i;
            }
            else
            {
                // Replace quote at first position:
                QByteArray first = QString("‘").toUtf8();
                line.replace(last_s, 1, first);

                // Replace quote at second position:
                line.replace(i + first.size() - 1, 1, QString("’").toUtf8());

                last_s = -1;
            }
        }
    }

    return line;
}

/*
 * Handles all the fancy text highlighting.
 */
static void insertText(QTextCursor &cursor, const QByteArray &line,
    const QTextCharFormat &format)
{
    if (line.isEmpty() ||

        // Render text normally if text decoration is disabled.
        !kristall::globals().options.enable_text_decoration ||

        // Render lines not containing asterisks/underscores normally.
        // This actually helps reduce the small overhead on large pages to
        // being almost negligable
        (!line.contains("*") && !line.contains("_")))
    {
        // Empty lines must be in standard format.
        cursor.insertText(line, format);
        return;
    }

    // Easier to work on this as an array of QChars
    QString text(line);

    // Whether to hide formatting codes (*, and _). This option
    // is mainly here so that the code which strips these is
    // more understandable.
    static const bool HIDE_FORMATS = true;

    // The first thing we do is convert double-asterisk bolding to single-asterisk.
    // This makes it A LOT easier to bold these things.
    //
    // This is done using this regex. In a simpler, pseudo form, it can be written as:
    // (punctuation/whitespace/line-begin)+\*\*(bolded text)\*\*(punctuation/whitespace/EOL)
    // Just stare at it a bit and you might figure out how it works...
    QRegularExpression BOLD_DBL_REGEX
        = QRegularExpression(R"((^|[\s.,!?[\]()\\-])+\*\*([^\*\s]+[^\*]+[^\*\s]+)\*\*($|[\s.,!?[\]()\\-]))");
    text.replace(BOLD_DBL_REGEX,  QString(R"(\1*\2*\3)"));

    QTextCharFormat fmt = format;
    bool bold = false, underline = false;
    bool was_bold = false, was_underline = false;
    int last = 0;

    // Used to prepare the format before actually drawing the text.
    auto format_text = [&bold, &underline, &was_bold, &was_underline, &last, &text, &fmt](int i) -> QString
    {
        // Makes sure that bold/underline text only gets printed
        // if it has a matching * or _.
        if (bold && !text.mid(i, text.length() - i).contains("*"))
            bold = false;
        if (underline && !text.mid(i, text.length() - i).contains("_"))
            underline = false;

        // Sets format to bold/underline as necessary.
        auto f = fmt.font();
        f.setBold(bold);
        fmt.setFont(f);
        fmt.setUnderlineStyle(underline ?
            QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline);

        // Remove formats
        QString span = text.mid(last, i - last);
        if (HIDE_FORMATS &&
            span.length() > 1 &&
            (((bold || was_bold) && span.startsWith("*")) ||
             ((underline || was_underline) && span.startsWith("_"))))
        {
            span = span.mid(1, span.length() - 1);
        }

        return span;
    };

    for (int i = 0; i < text.length(); ++i)
    {
        if (text[i] == '*')
        {
            // Format and insert the text.
            cursor.insertText(format_text(i), fmt);

            // 'Toggle' bold state.
            if (was_bold) was_bold = false;
            if (bold) {
                was_bold = true;
                bold = false;
            } else {
                // Only start bold formatting if this looks like bold formatting:
                // * Previous char must be either whitespace, nothing
                // * Next char must not be: whitespace, comma, full-stop, asterisk, or underscore.
                if ((i == 0 || text[i - 1].isSpace()) &&
                    (i + 1) < text.length() &&
                    !text[i + 1].isSpace() &&
                    text[i + 1] != ',' &&
                    text[i + 1] != '.' &&
                    text[i + 1] != '*' &&
                    text[i + 1] != '_')
                {
                    bold = true;
                }
            }

            last = i;
        }
        else if (text[i] == '_')
        {
            // Insert the text
            cursor.insertText(format_text(i), fmt);

            // 'Toggle' underline state.
            if (was_underline) was_underline = false;
            if (underline) {
                was_underline = true;
                underline = false;
            } else {
                // Only start underline formatting if it looks like an underline.
                // * Previous char must be either whitespace or nothing
                // * Next char must not be: whitespace, comma, full-stop, asterisk, or underscore.
                if ((i == 0 || text[i - 1].isSpace()) &&
                    (i + 1) < text.length() &&
                    !text[i + 1].isSpace() &&
                    text[i + 1] != ',' &&
                    text[i + 1] != '.' &&
                    text[i + 1] != '*' &&
                    text[i + 1] != '_')
                {
                    underline = true;
                }
            }

            last = i;
        }

        if (i == text.length() - 1)
        {
            QString span = text.mid(last, i - last + 1);

            // Skip if the span is just an asterisk/underline
            if (HIDE_FORMATS &&
                ((was_bold && span == "*") ||
                (was_underline && span == "_")))
            {
                break;
            }

            // Strips previous underline/asterisk
            if (HIDE_FORMATS &&
                span.length() > 1 &&
                ((was_bold && span.startsWith("*")) ||
                 (was_underline && span.startsWith("_"))))
            {
                span = span.mid(1, span.length() - 1);
            }

            // Draw ending text normally.
            cursor.insertText(span, format);
            break;
        }
    }
}
