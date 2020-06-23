#include "markdownrenderer.hpp"

#include "textstyleinstance.hpp"

#include <cmark.h>
#include <node.h>
#include <cassert>

#include <QDebug>
#include <QTextCursor>

//static char const *nodeToStr(cmark_node_type type)
//{
//    switch (type)
//    {
//    case CMARK_NODE_DOCUMENT:
//        return "CMARK_NODE_DOCUMENT";
//    case CMARK_NODE_BLOCK_QUOTE:
//        return "CMARK_NODE_BLOCK_QUOTE";
//    case CMARK_NODE_LIST:
//        return "CMARK_NODE_LIST";
//    case CMARK_NODE_ITEM:
//        return "CMARK_NODE_ITEM";
//    case CMARK_NODE_CODE_BLOCK:
//        return "CMARK_NODE_CODE_BLOCK";
//    case CMARK_NODE_HTML_BLOCK:
//        return "CMARK_NODE_HTML_BLOCK";
//    case CMARK_NODE_CUSTOM_BLOCK:
//        return "CMARK_NODE_CUSTOM_BLOCK";
//    case CMARK_NODE_PARAGRAPH:
//        return "CMARK_NODE_PARAGRAPH";
//    case CMARK_NODE_HEADING:
//        return "CMARK_NODE_HEADING";
//    case CMARK_NODE_THEMATIC_BREAK:
//        return "CMARK_NODE_THEMATIC_BREAK";
//    case CMARK_NODE_TEXT:
//        return "CMARK_NODE_TEXT";
//    case CMARK_NODE_SOFTBREAK:
//        return "CMARK_NODE_SOFTBREAK";
//    case CMARK_NODE_LINEBREAK:
//        return "CMARK_NODE_LINEBREAK";
//    case CMARK_NODE_CODE:
//        return "CMARK_NODE_CODE";
//    case CMARK_NODE_HTML_INLINE:
//        return "CMARK_NODE_HTML_INLINE";
//    case CMARK_NODE_CUSTOM_INLINE:
//        return "CMARK_NODE_CUSTOM_INLINE";
//    case CMARK_NODE_EMPH:
//        return "CMARK_NODE_EMPH";
//    case CMARK_NODE_STRONG:
//        return "CMARK_NODE_STRONG";
//    case CMARK_NODE_LINK:
//        return "CMARK_NODE_LINK";
//    case CMARK_NODE_IMAGE:
//        return "CMARK_NODE_IMAGE";
//    }
//    return "UNKNOWN";
//};

struct RenderState
{
    QTextCursor cursor;

    QUrl root_url;
    DocumentStyle const *style;
    DocumentOutlineModel *outline;
    TextStyleInstance text_style;

    bool suppress_next_block = false;

    void emitNewBlock() {
        if(not suppress_next_block) {
            cursor.insertBlock();
        }
        suppress_next_block = false;
    }
};

static void renderNode(RenderState &state, cmark_node const &node, QTextCharFormat current_format);

static void renderChildren(RenderState &state, cmark_node const & node, QTextCharFormat current_format)
{
    for (auto child = node.first_child; child != nullptr; child = child->next)
    {
        renderNode(state, *child, current_format);
    }
}

/* Leaf Nodes
* * CMARK_NODE_HTML_BLOCK
* * CMARK_NODE_THEMATIC_BREAK
* * CMARK_NODE_CODE_BLOCK
* * CMARK_NODE_TEXT
* * CMARK_NODE_SOFTBREAK
* * CMARK_NODE_LINEBREAK
* * CMARK_NODE_CODE
* * CMARK_NODE_HTML_INLINE
*/

static QString extractNodeText(cmark_node const &node)
{
    return QString::fromUtf8((char const*)node.data, node.len);
}

static void renderNode(RenderState &state, cmark_node const & node, QTextCharFormat current_format)
{
    auto & cursor = state.cursor;

    switch (node.type)
    {
    case CMARK_NODE_DOCUMENT:
    {
        renderChildren(state, node, current_format);
        break;
    }

    case CMARK_NODE_BLOCK_QUOTE:
    {
        state.emitNewBlock();
        state.suppress_next_block = true;

        cursor.setBlockFormat(state.text_style.block_quote_format);
        renderChildren(state, node, current_format);

        state.emitNewBlock();
        state.suppress_next_block = true;
        cursor.setBlockFormat(state.text_style.standard_format);

        break;
    }
    case CMARK_NODE_LIST:
    {
        auto fmt = cursor.blockFormat();

        if(node.as.list.list_type == CMARK_BULLET_LIST) {
            cursor.insertList(QTextListFormat::ListDisc);
        } else {
            cursor.insertList(QTextListFormat::ListDecimal);
        }

        state.suppress_next_block = true;
        renderChildren(state, node, current_format);

        state.emitNewBlock();
        state.suppress_next_block = true;
        cursor.setBlockFormat(fmt);
        break;
    }
    case CMARK_NODE_ITEM:
    {
        renderChildren(state, node, current_format);
        break;
    }
    case CMARK_NODE_CODE_BLOCK:
    {
        state.emitNewBlock();
        cursor.setBlockFormat(state.text_style.preformatted_format);

        QString code = extractNodeText(node);
        cursor.insertText(code, state.text_style.preformatted);

        state.emitNewBlock();
        state.suppress_next_block = true;
        cursor.setBlockFormat(state.text_style.standard_format);

        break;
    }
    case CMARK_NODE_HTML_BLOCK:
    {
        qDebug() << "CMARK_NODE_HTML_BLOCK";
        break;
    }
    case CMARK_NODE_CUSTOM_BLOCK:
    {
        qDebug() << "CMARK_NODE_CUSTOM_BLOCK";
        break;
    }
    case CMARK_NODE_PARAGRAPH:
    {
        state.emitNewBlock();
        renderChildren(state, node, current_format);
        break;
    }
    case CMARK_NODE_HEADING:
    {
        QTextCharFormat fmt = current_format;
        state.emitNewBlock();
        switch(node.as.heading.level) {
        case 1: fmt = state.text_style.standard_h1; break;
        case 2: fmt = state.text_style.standard_h2; break;
        case 3: fmt = state.text_style.standard_h3; break;
        case 4: fmt = state.text_style.standard_h3; break;
        case 5: fmt = state.text_style.standard_h3; break;
        case 6: fmt = state.text_style.standard_h3; break;

        default: qDebug() << "heading" << node.as.heading.level; break;
        }

        switch(node.as.heading.level) {
        case 1: state.outline->appendH1("Unknown H1", QString { }); break;
        case 2: state.outline->appendH2("Unknown H2", QString { }); break;
        case 3: state.outline->appendH3("Unknown H3", QString { }); break;
        }

        renderChildren(state, node, fmt);
        break;
    }
    case CMARK_NODE_THEMATIC_BREAK:
    {
        qDebug() << "CMARK_NODE_THEMATIC_BREAK";
        break;
    }
    case CMARK_NODE_TEXT:
    {
        cursor.insertText(extractNodeText(node), current_format);
        break;
    }
    case CMARK_NODE_SOFTBREAK:
    {
        // qDebug() << "CMARK_NODE_SOFTBREAK";
        cursor.insertText(" ", current_format);
        break;
    }
    case CMARK_NODE_LINEBREAK:
    {
        // qDebug() << "CMARK_NODE_LINEBREAK";
        state.emitNewBlock();
        break;
    }
    case CMARK_NODE_CODE:
    {
        QString code = extractNodeText(node);
        cursor.insertText(code, state.text_style.preformatted);
        break;
    }
    case CMARK_NODE_HTML_INLINE:
    {
        qDebug() << "CMARK_NODE_HTML_INLINE";
        break;
    }
    case CMARK_NODE_CUSTOM_INLINE:
    {
        qDebug() << "CMARK_NODE_CUSTOM_INLINE";
        break;
    }
    case CMARK_NODE_EMPH:
    {
        auto fmt = current_format;
        fmt.setFontItalic(true);
        renderChildren(state, node, fmt);
        break;
    }
    case CMARK_NODE_STRONG:
    {
        auto fmt = current_format;
        fmt.setFontWeight(QFont::Bold);
        renderChildren(state, node, fmt);
        break;
    }
    case CMARK_NODE_LINK:
    {
        auto fmt = state.text_style.standard_link;
        fmt.setAnchorHref(QString::fromUtf8((char*)node.as.link.url));
        renderChildren(state, node, fmt);
        break;
    }
    case CMARK_NODE_IMAGE:
    {
        qDebug() << "CMARK_NODE_IMAGE";
        break;
    }
    }
}

std::unique_ptr<QTextDocument> MarkdownRenderer::render(
    QByteArray const &input,
    QUrl const &root_url,
    DocumentStyle const &style,
    DocumentOutlineModel &outline)
{

    std::unique_ptr<cmark_node, decltype(&cmark_node_free)> md_root{
        cmark_parse_document(input.data(), input.size(), 0),
        &cmark_node_free,
    };
    if (not md_root)
        return nullptr;

    auto doc = std::make_unique<QTextDocument>();
    doc->setDocumentMargin(style.margin);
    doc->setIndentWidth(20);

    outline.beginBuild();

    RenderState state = {
        QTextCursor { doc.get() },
        root_url,
        &style,
        &outline,
        TextStyleInstance { style },
    };

    renderNode(state, *md_root, state.text_style.standard);

    outline.endBuild();

    return doc;
}
