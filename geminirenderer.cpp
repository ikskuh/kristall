#include "geminirenderer.hpp"

#include <QTextList>
#include <QTextBlock>

static QByteArray trim_whitespace(QByteArray items)
{
    int start = 0;
    while(start < items.size() and isspace(items.at(start))) {
        start += 1;
    }
    int end = items.size() - 1;
    while(end > 0 and isspace(items.at(end))) {
        end -= 1;
    }
    return items.mid(start, end - start + 1);
}

GeminiStyle::GeminiStyle() :
    standard_font(),
    h1_font(),
    h2_font(),
    h3_font(),
    preformatted_font(),
    background_color(0xFF, 0xFF, 0xFF),
    standard_color(0x00, 0x00, 0x00),
    h1_color(0xFF, 0x00, 0x00),
    h2_color(0x00, 0x80, 0x00),
    h3_color(0x80, 0xFF, 0x00),
    internal_link_color(0x00, 0x80, 0x0FF),
    external_link_color(0x00, 0x00, 0xFF),
    cross_scheme_link_color(0x80, 0x00, 0xFF),
    internal_link_prefix("→ "),
    external_link_prefix("⇒ ")
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

GeminiRenderer::GeminiRenderer(GeminiStyle const & _style) :
    style(_style)
{

}

std::unique_ptr<QTextDocument> GeminiRenderer::render(const QByteArray &input, QUrl const & root_url, DocumentOutlineModel &outline)
{

    QTextCharFormat preformatted;
    preformatted.setFont(style.preformatted_font);

    QTextCharFormat standard;
    standard.setFont(style.standard_font);

    QTextCharFormat standard_link;
    standard_link.setFont(style.standard_font);
    standard_link.setForeground(QBrush(style.internal_link_color));

    QTextCharFormat external_link;
    external_link.setFont(style.standard_font);
    external_link.setForeground(QBrush(style.external_link_color));

    QTextCharFormat cross_protocol_link;
    cross_protocol_link.setFont(style.standard_font);
    cross_protocol_link.setForeground(QBrush(style.cross_scheme_link_color));

    QTextCharFormat standard_h1;
    standard_h1.setFont(style.h1_font);
    standard_h1.setForeground(QBrush(style.h1_color));

    QTextCharFormat standard_h2;
    standard_h2.setFont(style.h2_font);
    standard_h2.setForeground(QBrush(style.h2_color));

    QTextCharFormat standard_h3;
    standard_h3.setFont(style.h3_font);
    standard_h3.setForeground(QBrush(style.h3_color));

    std::unique_ptr<QTextDocument> result = std::make_unique<QTextDocument>();
    result->setDocumentMargin(55.0);

    QTextCursor cursor { result.get() };

    QTextBlockFormat non_list_format = cursor.blockFormat();

    bool verbatim = false;
    QTextList * current_list = nullptr;

    outline.beginBuild();

    QList<QByteArray> lines = input.split('\n');
    for(auto const & line : lines)
    {
        if(verbatim) {
            if(line.startsWith("```")) {
                verbatim = false;
            }
            else {
                cursor.setCharFormat(preformatted);
                cursor.insertText(line + "\n");
            }
        } else {
            if(line.startsWith("*")) {
                if(current_list == nullptr) {
                    cursor.deletePreviousChar();
                    current_list = cursor.insertList(QTextListFormat::ListDisc);
                } else {
                    cursor.insertBlock();
                }

                QString item = trim_whitespace(line.mid(1));

                cursor.insertText(item, standard);
                continue;
            } else {
                if(current_list != nullptr) {
                    cursor.insertBlock();
                    cursor.setBlockFormat(non_list_format);
                }
                current_list = nullptr;
            }

            if(line.startsWith("###")) {
                auto heading = trim_whitespace(line.mid(3));

                cursor.insertText(heading + "\n", standard_h3);
                outline.appendH3(heading);
            }
            else if(line.startsWith("##")) {
                auto heading = trim_whitespace(line.mid(2));

                cursor.insertText(heading + "\n", standard_h2);
                outline.appendH2(heading);
            }
            else if(line.startsWith("#")) {
                auto heading = trim_whitespace(line.mid(1));

                cursor.insertText(heading + "\n", standard_h1);
                outline.appendH1(heading);
            }
            else if(line.startsWith("=>")) {
                auto const part = line.mid(2).trimmed();

                QByteArray link, title;

                int index = -1;
                for(int i = 0; i < part.size(); i++) {
                    if(isspace(part[i])) {
                        index = i;
                        break;
                    }
                }

                if(index > 0) {
                    link  = trim_whitespace(part.mid(0, index));
                    title = trim_whitespace(part.mid(index + 1));
                } else {
                    link  = trim_whitespace(part);
                    title = trim_whitespace(part);
                }

                auto local_url = QUrl(link);

                auto absolute_url = root_url.resolved(QUrl(link));

                // qDebug() << link << title;

                auto fmt = standard_link;
                if(not local_url.isRelative()) {
                    fmt = external_link;
                }

                QString suffix = "";
                if(absolute_url.scheme() != root_url.scheme()) {
                    suffix = " [" + absolute_url.scheme().toUpper() + "]";
                    fmt = cross_protocol_link;
                }

                QString prefix = "";
                if(local_url.isRelative() or absolute_url.host() == root_url.host()) {
                    prefix = style.internal_link_prefix;
                } else {
                    prefix = style.external_link_prefix;
                }

                fmt.setAnchor(true);
                fmt.setAnchorHref(absolute_url.toString());
                cursor.insertText(prefix + title + suffix + "\n", fmt);
            }
            else if(line.startsWith("```")) {
                verbatim = true;
            }
            else {
                cursor.insertText(line + "\n", standard);
            }
        }
    }

    outline.endBuild();
    return result;
}
