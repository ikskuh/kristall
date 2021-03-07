#include "htmlrenderer.hpp"

#include "renderhelpers.hpp"
#include "textstyleinstance.hpp"
#include "gumbo.h"

#include <QDebug>
#include <QTextTable>
#include <QRegularExpression>

static void* malloc_wrapper(void*, size_t size) { return malloc(size); }

static void free_wrapper(void*, void* ptr) { free(ptr); }

static GumboOptions const gumbo_options = {
    &malloc_wrapper, &free_wrapper, // memory management
    nullptr,                        // user pointer
    4,                              // tab width
    false,                          // stop on first error
    -1,                             // maximum numbers of errors (-1 = infinite)
    GUMBO_TAG_LAST,
    GUMBO_NAMESPACE_HTML
};

static void destroyGumboOutput(GumboOutput * output)
{
    gumbo_destroy_output(&gumbo_options, output);
}

static const char* find_title(const GumboNode* root) {
    assert(root->type == GUMBO_NODE_ELEMENT);
    if(root->v.element.children.length < 2)
        return nullptr;

    const GumboVector* root_children = &root->v.element.children;
    GumboNode* head = nullptr;
    for (size_t i = 0; i < root_children->length; ++i) {
        GumboNode* child = (GumboNode*)root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT and child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    if(head == nullptr)
        return nullptr;

    GumboVector* head_children = &head->v.element.children;
    for (size_t i = 0; i < head_children->length; ++i) {
        GumboNode* child = (GumboNode*)head_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT and child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                return "";
            }
            GumboNode* title_text = (GumboNode*)child->v.element.children.data[0];
            if(title_text->type == GUMBO_NODE_TEXT or title_text->type == GUMBO_NODE_WHITESPACE)
                return title_text->v.text.text;
            return nullptr;
        }
    }
    return nullptr;
}

struct RenderState
{
    QTextCursor cursor;
    TextStyleInstance text_style;
    QUrl root_url;
    DocumentStyle const * style;
    DocumentOutlineModel * outline;
};

static char const * getAttribute(GumboElement const & element, char const * attrib_name)
{
    for(size_t i = 0; i < element.attributes.length; i++)
    {
        auto const attrib = static_cast<GumboAttribute const *>(element.attributes.data[i]);
        if(strcmp(attrib->name, attrib_name) == 0)
            return attrib->value;
    }
    return nullptr;
}

struct TextFormatReset
{
    QTextCursor * cursor;

    QTextCharFormat char_format;
    QTextBlockFormat block_format;

    TextFormatReset(QTextCursor * cursor) :
        cursor(cursor),
        char_format(cursor->charFormat()),
        block_format(cursor->blockFormat())
    {

    }

    TextFormatReset(TextFormatReset const &) = delete;
    TextFormatReset(TextFormatReset &&) = delete;

    ~TextFormatReset()
    {
        this->cursor->setCharFormat(this->char_format);
        this->cursor->setBlockFormat(this->block_format);
    }

};

// Problems:
// Style/theme elements must use a push/pop
// use instead of "replacing" styles
// Otherwise, <h1><a>Foo</a></h1> will be rendered as a link, not as a heading.
// Should be combined here.

static void renderRecursive(RenderState & state, GumboNode const & node, int nesting = 0)
{
    auto & cursor = state.cursor;
    auto & text_style = state.text_style;
    switch(node.type)
    {
    /** Document node.  v will be a GumboDocument. */
    case GUMBO_NODE_DOCUMENT: {
        qWarning() << "Detected embedded document";
    }

    /** Element node.  v will be a GumboElement. */
    case GUMBO_NODE_ELEMENT: {
        auto const & element = node.v.element;

        TextFormatReset format_reset { &cursor };

        // qDebug() << "begin node(" << gumbo_normalized_tagname(element.tag) << ")";

        switch(element.tag) {

        // Stripped tags
        case GUMBO_TAG_STYLE:
        case GUMBO_TAG_SCRIPT:
            return;

        case GUMBO_TAG_NAV: {
            // TODO: Optionally strip navigation from sites
            if(true)
                return;
            break;
        }

        // Terminal tags
        case GUMBO_TAG_IMG: {
            // TODO: Insert link to image here
            cursor.insertText("[IMG]");
            return;
        }
        case GUMBO_TAG_SVG: {
            // TODO: Insert link to image here
            cursor.insertText("[SVG]");
            return;
        }
        case GUMBO_TAG_BUTTON: {
            // TODO: Insert link to image here
            cursor.insertText("[BUTTON]");
            return;
        }
        case GUMBO_TAG_INPUT: {
            // TODO: Insert link to image here
            cursor.insertText("[INPUT]");
            return;
        }

        // Paragraph-like elements:
        case GUMBO_TAG_DIV: // <div> is the same as <p> for us
        case GUMBO_TAG_P: {
            // cursor.insertBlock();
            break;
        }
        case GUMBO_TAG_H1: {
            // cursor.insertBlock();
            cursor.setBlockFormat(text_style.heading_format);
            cursor.setCharFormat(text_style.standard_h1);
            break;
        }
        case GUMBO_TAG_H2: {
            // cursor.insertBlock();
            cursor.setBlockFormat(text_style.heading_format);
            cursor.setCharFormat(text_style.standard_h2);

            break;
        }
        case GUMBO_TAG_H3: {
            // cursor.insertBlock();
            cursor.setBlockFormat(text_style.heading_format);
            cursor.setCharFormat(text_style.standard_h3);
            break;
        }

        case GUMBO_TAG_PRE: {
            // cursor.insertBlock();
            cursor.setBlockFormat(text_style.preformatted_format);
            cursor.setCharFormat(text_style.preformatted);
            break;
        }

        case GUMBO_TAG_OL:
        case GUMBO_TAG_UL: {
            // cursor.insertBlock();

            if(element.tag == GUMBO_TAG_OL) {
                auto fmt = text_style.list_format;
                fmt.setStyle(QTextListFormat::ListDecimal);
                fmt.setNumberPrefix("");
                fmt.setNumberSuffix(".");
                cursor.createList(fmt);
            }
            else {
                cursor.createList(text_style.list_format);
            }
            break;
        }
        case GUMBO_TAG_LI: {
            break;
        }
        case GUMBO_TAG_BLOCKQUOTE: {
            QTextTable *table = cursor.insertTable(1, 1,text_style.blockquote_tableformat);
            cursor.setBlockFormat(text_style.blockquote_format);
            QTextTableCell cell = table->cellAt(0, 0);
            cell.setFormat(text_style.blockquote);

            cursor.setCharFormat(text_style.blockquote);

            break;
        }

        // Text modification elements:
        case GUMBO_TAG_SPAN: {
            // This usually has a style change, but we ignore that completly
            break;
        }
        case GUMBO_TAG_BR: {
            cursor.insertText("\n");
            break;
        }
        case GUMBO_TAG_I: {
            auto fmt = cursor.charFormat();
            fmt.setFontItalic(true);
            cursor.setCharFormat(fmt);
            break;
        }
        case GUMBO_TAG_B: {
            auto fmt = cursor.charFormat();
            fmt.setFontWeight(QFont::Bold);
            cursor.setCharFormat(fmt);
            break;
        }
        case GUMBO_TAG_U: {
            auto fmt = cursor.charFormat();
            fmt.setFontUnderline(true);
            cursor.setCharFormat(fmt);
            break;
        }
        case GUMBO_TAG_A: {
            char const * anchor = getAttribute(element, "href");
            if(anchor == nullptr) {
                anchor = "#";
            }

            auto fmt = text_style.standard_link;
            fmt.setAnchor(true);
            fmt.setAnchorHref(QString::fromUtf8(anchor));
            cursor.setCharFormat(fmt);
            break;
        }
        default:
            qDebug() << "unhandled tag:" << gumbo_normalized_tagname(element.tag);
            break;
        }

        for (size_t i = 0; i < element.children.length; ++i) {
            GumboNode* child = (GumboNode*)element.children.data[i];
            renderRecursive(state, *child, nesting + 1);
        }

        switch(element.tag) {
        // case GUMBO_TAG_PRE: {
//            // Set the last line of the preformatted block to have
//            // standard line height.
//            QTextBlockFormat fmt = cursor.blockFormat();
//            fmt.setLineHeight(state.style->line_height_p, QTextBlockFormat::LineDistanceHeight);
//            cursor.movePosition(QTextCursor::PreviousBlock);
//            cursor.setBlockFormat(fmt);

//            cursor.movePosition(QTextCursor::NextBlock);
//            break;
//        }

        // Requires closing block
        case GUMBO_TAG_PRE:
        case GUMBO_TAG_P:
        case GUMBO_TAG_DIV:
        case GUMBO_TAG_H1:
        case GUMBO_TAG_H2:
        case GUMBO_TAG_H3:
            cursor.insertBlock();
            break;

        case GUMBO_TAG_OL:
        case GUMBO_TAG_UL:
            // cursor.insertBlock();
            break;

        case GUMBO_TAG_LI:
            // Terminate the <li> by pressing "enter"
            cursor.insertBlock();
            break;

        case GUMBO_TAG_BLOCKQUOTE:
            cursor.deletePreviousChar();
            cursor.movePosition(QTextCursor::NextBlock);
            break;

        default: break;
        }

        // qDebug() << "end node(" << gumbo_normalized_tagname(element.tag) << ")";

        break;
    }

    /** Text node.  v will be a GumboText. */
    case GUMBO_NODE_TEXT: {
        auto const & text = node.v.text;

        auto contents = QString::fromUtf8(text.text);
        // qDebug() << contents;

        QRegularExpression regex { "\\s+", QRegularExpression::DotMatchesEverythingOption };

        // TODO: This is not quite right, but QTextCursor::inserText
        // will insert spurious blocks when a "\n" is encountered.
        state.cursor.insertText(contents.replace(regex, " "));
        break;
    }

    /** CDATA node. v will be a GumboText. */
    case GUMBO_NODE_CDATA: {
        auto const & text = node.v.text;

        auto const contents = QString::fromUtf8(text.text);

        // TODO: This is not quite right, but QTextCursor::inserText
        // will insert spurious blocks when a "\n" is encountered.
        state.cursor.insertText(contents.trimmed());
        break;
    }

    /** Comment node.  v will be a GumboText, excluding comment delimiters. */
    case GUMBO_NODE_COMMENT: {
        // qDebug() << "comment(" << ")";
        break;
    }

    /** Text node, where all contents is whitespace.  v will be a GumboText. */
    case GUMBO_NODE_WHITESPACE: {
        // qDebug() << "whitespace(" << ")";
        break;
    }

    /** Template node.  This is separate from GUMBO_NODE_ELEMENT because many
     * client libraries will want to ignore the contents of template nodes, as
     * the spec suggests.  Recursing on GUMBO_NODE_ELEMENT will do the right thing
     * here, while clients that want to include template contents should also
     * check for GUMBO_NODE_TEMPLATE.  v will be a GumboElement.  */
    case GUMBO_NODE_TEMPLATE: {
        qDebug() << "template(" << "???" << ")";
        break;
    }
    }
}

std::unique_ptr<QTextDocument> HtmlRenderer::render(
        QByteArray const &input,
        QUrl const & root_url,
        DocumentStyle const & style,
        DocumentOutlineModel & outline,
        QString & page_title)
{
    std::unique_ptr<GumboOutput, decltype(&destroyGumboOutput)> gumbo_output {
        gumbo_parse_with_options(&gumbo_options, input.data(), input.length()),
        &destroyGumboOutput,
    };

    if(gumbo_output->errors.length > 0) {
        qDebug() << "Parsing the html document yielded" << gumbo_output->errors.length << "errors!";
    }

    if(gumbo_output->root->type != GUMBO_NODE_ELEMENT) {
        qWarning() << "html document has no proper root node!";
        return nullptr;
    }

    auto doc = std::make_unique<QTextDocument>();
    renderhelpers::setPageMargins(doc.get(), style.margin_h, style.margin_v);
    doc->setIndentWidth(style.indent_size);

    outline.beginBuild();

    // Find page title
    {
        const char* title = find_title(gumbo_output->root);
        if(title != nullptr) {
            page_title = QString::fromUtf8(title);
        }
    }

    {
        GumboVector const * const root_children = &gumbo_output->root->v.element.children;
        GumboNode* body = nullptr;
        for (size_t i = 0; i < root_children->length; ++i) {
            GumboNode* child = (GumboNode*)root_children->data[i];
            if (child->type == GUMBO_NODE_ELEMENT and child->v.element.tag == GUMBO_TAG_BODY) {
                body = child;
                break;
            }
        }
        if(body != nullptr)
        {
            RenderState state {
                QTextCursor { doc.get() },
                TextStyleInstance { style },
                root_url,
                &style,
                &outline,
            };

            state.cursor.setBlockFormat(state.text_style.standard_format);
            state.cursor.setCharFormat(state.text_style.standard);

            renderRecursive(state, *body);
        }
    }


    outline.endBuild();

    return doc;
}
