#include "gophermaprenderer.hpp"
#include "renderhelpers.hpp"
#include <cassert>
#include <QTextList>
#include <QTextBlock>
#include <QList>
#include <QStringList>
#include <QTextImageFormat>

#include <QDebug>
#include <QImage>

#include "kristall.hpp"


std::unique_ptr<QTextDocument> GophermapRenderer::render(const QByteArray &input, const QUrl &root_url, const DocumentStyle &themed_style)
{
    QTextCharFormat standard;
    standard.setFont(themed_style.preformatted_font);
    standard.setForeground(themed_style.preformatted_color);

    QTextCharFormat standard_link;
    standard_link.setFont(themed_style.preformatted_font);
    standard_link.setForeground(QBrush(themed_style.internal_link_color));

    QTextCharFormat external_link;
    external_link.setFont(themed_style.standard_font);
    external_link.setForeground(QBrush(themed_style.external_link_color));

    bool emit_text_only = (kristall::globals().options.gophermap_display == GenericSettings::PlainText);

    std::unique_ptr<QTextDocument> result = std::make_unique<QTextDocument>();
    renderhelpers::setPageMargins(result.get(), themed_style.margin_h, themed_style.margin_v);

    if(not emit_text_only)
    {
        QString icon_prefix;
        if(themed_style.background_color.valueF() < 0.65)
            icon_prefix = ":/icons/dark/gopher/";
        else
            icon_prefix = ":/icons/light/gopher/";
        qDebug() << "lightness" << themed_style.background_color.lightnessF();
        qDebug() << "value" << themed_style.background_color.valueF();

        result->addResource(QTextDocument::ImageResource, QUrl("gopher/binary"), QVariant::fromValue(QImage(icon_prefix + "binary.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/directory"), QVariant::fromValue(QImage(icon_prefix + "directory.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/dns"), QVariant::fromValue(QImage(icon_prefix + "dns.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/error"), QVariant::fromValue(QImage(icon_prefix + "error.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/gif"), QVariant::fromValue(QImage(icon_prefix + "gif.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/html"), QVariant::fromValue(QImage(icon_prefix + "html.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/image"), QVariant::fromValue(QImage(icon_prefix + "image.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/mirror"), QVariant::fromValue(QImage(icon_prefix + "mirror.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/search"), QVariant::fromValue(QImage(icon_prefix + "search.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/sound"), QVariant::fromValue(QImage(icon_prefix + "sound.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/telnet"), QVariant::fromValue(QImage(icon_prefix + "telnet.svg")));
        result->addResource(QTextDocument::ImageResource, QUrl("gopher/text"), QVariant::fromValue(QImage(icon_prefix + "text.svg")));
    }

    QTextCursor cursor{result.get()};

    QTextBlockFormat non_list_format = cursor.blockFormat();

    QTextCharFormat text_fmt = standard;

    char last_type = '1';

    QList<QByteArray> lines = input.split('\n');
    for (auto const &line : lines)
    {
        if (line.length() < 2) // skip lines without
            continue;

        if (line[line.size() - 1] != '\r')
            continue;

        auto items = line.mid(1, line.length() - 2).split('\t');
        if (items.size() < 2) // invalid
            continue;

        QString icon;
        QString scheme = "gopher";

        auto type = line.at(0);
        switch (type)
        {
        case '0': // Text File
            icon = "text";
            break;
        case '1': // Gopher submenu or link to another gopher server
            icon = "directory";
            break;
        case '2': // CCSO Nameserver
            icon = "dns";
            break;
        case '3': // Error code returned by a Gopher server to indicate failure
            icon = "error";
            break;
        case '4': // BinHex-encoded file (primarily for Macintosh computers)
            icon = "binary";
            break;
        case '5': // DOS file
            icon = "binary";
            break;
        case '6': // uuencoded file
            icon = "binary";
            break;
        case '7': // Gopher full-text search
            icon = "search";
            break;
        case '8': // Telnet
            icon = "telnet";
            scheme = "telnet";
            break;
        case '9': // Binary file
            icon = "binary";
            break;
        case '+': // Mirror or alternate server (for load balancing or in case of primary server downtime)
            icon = "mirror";
            break;
        case 'g': // GIF file
            icon = "gif";
            break;
        case 'I': // Image file
            icon = "image";
            break;
        case 'T': // Telnet 3270
            icon = "telnet";
            scheme = "telnet";
            break;
        //Non-Canonical Types
        case 'h': // HTML file
            icon = "html";
            break;
        case 'i': // Informational message
            icon = "informational";
            break;
        case 's': // Sound file
            icon = "sound";
            break;
        default: // unknown
            continue;
        }
        if(type == '+') {
            type = last_type;
        } else {
            last_type = type;
        }

        QString title = items.at(0);

        if (type == 'i')
        {
            const QString escapeRenderInput = title + "\n";
            renderhelpers::renderEscapeCodes(escapeRenderInput.toUtf8(), text_fmt, standard, cursor);
        }
        else
        {
            QString dst_url;

            // If a resource’s link starts with “URL:”, it is a direct link (to HTTP or another protocol), rather than a file or directory on this server.
            if (items.size() >= 2 && items.at(1).left(4) == "URL:")
            {
                auto item1 = QString(items.at(1));
                item1.remove(0, 4);
                dst_url = item1;
            }
            else
            {
                switch (items.size())
                {
                case 0:
                    assert(false);
                case 1:
                    assert(false);
                case 2:
                    dst_url = root_url.resolved(QUrl(items.at(1))).toString();
                    break;
                case 3:
                    dst_url = scheme + "://" + items.at(2) + "/" + QString(type) + items.at(1);
                    break;
                default:
                    dst_url = scheme + "://" + items.at(2) + ":" + items.at(3) + "/" + QString(type) + items.at(1);
                    break;
                }
            }

            if (not QUrl(dst_url).isValid())
            {
                // invlaid URL generated
                qDebug() << line << dst_url;
            }

            if(emit_text_only)
            {
                cursor.insertText("[" + icon + "] ", standard);
            }
            else
            {
                QTextImageFormat icon_fmt;
                icon_fmt.setFont(themed_style.preformatted_font);
                icon_fmt.setName(QString("gopher/%1").arg(icon));
                icon_fmt.setVerticalAlignment(QTextImageFormat::AlignTop);

                cursor.insertImage(icon_fmt);
                cursor.insertText(" ");
            }

            QTextCharFormat fmt = standard_link;
            fmt.setAnchor(true);
            fmt.setAnchorHref(dst_url);
            cursor.insertText(title + "\n", fmt);
        }
    }

    return result;
}
