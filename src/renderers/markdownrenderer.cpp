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
};

static void renderNode(RenderState &state, cmark_node const &node);

static void renderChildren(RenderState &state, cmark_node const & node)
{
    for (auto child = node.first_child; child != nullptr; child = child->next)
    {
        renderNode(state, *child);
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

static void renderNode(RenderState &state, cmark_node const & node)
{
    auto & cursor = state.cursor;

    switch (node.type)
    {
    case CMARK_NODE_DOCUMENT:
    {
        renderChildren(state, node);
        break;
    }

    case CMARK_NODE_BLOCK_QUOTE:
    {
        qDebug() << "CMARK_NODE_BLOCK_QUOTE";
        break;
    }
    case CMARK_NODE_LIST:
    {
        auto fmt = cursor.blockFormat();
        cursor.insertList(QTextListFormat::ListDisc);
        renderChildren(state, node);
        cursor.setBlockFormat(fmt);
        break;
    }
    case CMARK_NODE_ITEM:
    {
        renderChildren(state, node);
        break;
    }
    case CMARK_NODE_CODE_BLOCK:
    {
        qDebug() << "CMARK_NODE_CODE_BLOCK";
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
        cursor.insertBlock();
        renderChildren(state, node);
        break;
    }
    case CMARK_NODE_HEADING:
    {
        cursor.insertBlock();
        switch(node.as.heading.level) {
        case 1: cursor.setCharFormat(state.text_style.standard_h1); break;
        case 2: cursor.setCharFormat(state.text_style.standard_h2); break;
        case 3: cursor.setCharFormat(state.text_style.standard_h3); break;
        case 4: cursor.setCharFormat(state.text_style.standard_h3); break;
        case 5: cursor.setCharFormat(state.text_style.standard_h3); break;
        case 6: cursor.setCharFormat(state.text_style.standard_h3); break;

        default: qDebug() << "heading" << node.as.heading.level; break;
        }

        switch(node.as.heading.level) {
        case 1: state.outline->appendH1("Unknown H1", QString { }); break;
        case 2: state.outline->appendH2("Unknown H2", QString { }); break;
        case 3: state.outline->appendH3("Unknown H3", QString { }); break;
        }

        renderChildren(state, node);
        cursor.setCharFormat(state.text_style.standard);
        break;
    }
    case CMARK_NODE_THEMATIC_BREAK:
    {
        qDebug() << "CMARK_NODE_THEMATIC_BREAK";
        break;
    }
    case CMARK_NODE_TEXT:
    {
        cursor.insertText(extractNodeText(node));
        break;
    }
    case CMARK_NODE_SOFTBREAK:
    {
        qDebug() << "CMARK_NODE_SOFTBREAK";
        break;
    }
    case CMARK_NODE_LINEBREAK:
    {
        qDebug() << "CMARK_NODE_LINEBREAK";
        break;
    }
    case CMARK_NODE_CODE:
    {
        qDebug() << "CMARK_NODE_CODE";
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
        qDebug() << "CMARK_NODE_EMPH";
        break;
    }
    case CMARK_NODE_STRONG:
    {
        qDebug() << "CMARK_NODE_STRONG";
        break;
    }
    case CMARK_NODE_LINK:
    {
        // TODO: Implementing linking
        // cursor.insertText(QString::fromUtf8((char*)node.as.link.title));
        // qDebug() << "CMARK_NODE_LINK" << (char*)node.as.link.url;
        renderChildren(state, node);
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

    qDebug() << md_root.get();

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

    renderNode(state, *md_root);

    outline.endBuild();

    return doc;
}
