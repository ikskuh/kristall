#include "htmlrenderer.hpp"

#include "renderhelpers.hpp"
#include "textstyleinstance.hpp"
#include "gumbo.h"

#include <QDebug>
#include <QTextTable>
#include <QTextList>
#include <QRegularExpression>

#include <assert.h>

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
    QString & stream;
    QUrl root_url;
    DocumentOutlineModel & outline;

    //! when non-null, we're inside a header element and accumulate the text to
    //! compute the outline.
    QString * header_text;

    int header_count;
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

static void renderRecursive(RenderState & state, GumboNode const & node, int nesting = 0)
{
    auto & stream = state.stream;
    auto & outline = state.outline;
    switch(node.type)
    {
    /** Document node.  v will be a GumboDocument. */
    case GUMBO_NODE_DOCUMENT: {
        qWarning() << "Detected embedded document";
        break;
    }

    /** Element node.v will be a GumboElement. */
    case GUMBO_NODE_ELEMENT: {
        auto const & element = node.v.element;

        // qDebug() << "begin node(" << gumbo_normalized_tagname(element.tag) << ")";

        bool process_header = false;
        QString header_text;

        switch(element.tag) {

        // Stripped tags
        case GUMBO_TAG_STYLE:
        case GUMBO_TAG_SCRIPT:
        case GUMBO_TAG_UNKNOWN:
            return;

        case GUMBO_TAG_BR:
            stream += "<br>";
            return;

        case GUMBO_TAG_HR:
            // HACK: stream += "<p style=\"text-align: center; width: 100%;\"><u>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</u><br>&nbsp;</p>";
            stream += "<hr>";
            return;

        case GUMBO_TAG_NAV: {
            // TODO: Optionally strip navigation from sites
            //if(true)
            //    return;
            stream += "<nav>";
            break;
        }

        // Terminal tags
        case GUMBO_TAG_IMG: {
            //stream += "[IMG]";
            return;
        }
        case GUMBO_TAG_SVG: {
            //stream += "[SVG]";
            return;
        }
        case GUMBO_TAG_BUTTON: {
            //stream += "[BUTTON]";
            return;
        }
        case GUMBO_TAG_INPUT: {
            //stream += "[INPUT]";
            return;
        }

        case GUMBO_TAG_A: {
            char const * anchor = getAttribute(element, "href");
            if(anchor == nullptr) {
                anchor = "#";
            }
            stream += "<a href=\""+QString::fromUtf8(anchor)+"\">";
            break;
        }

        case GUMBO_TAG_H1:
        case GUMBO_TAG_H2:
        case GUMBO_TAG_H3:
        case GUMBO_TAG_H4:
        case GUMBO_TAG_H5:
        case GUMBO_TAG_H6:
            if(state.header_text == nullptr) {
                process_header = true;
                state.header_text = &header_text;
            }
            stream += "<" + QString::fromUtf8(gumbo_normalized_tagname(element.tag)) + QString(" id=\"header-%1\">").arg(state.header_count);
            break;

        default:
            stream += "<" + QString::fromUtf8(gumbo_normalized_tagname(element.tag)) + ">";
            break;
        }

        for (size_t i = 0; i < element.children.length; ++i) {
            GumboNode* child = (GumboNode*)element.children.data[i];
            renderRecursive(state, *child, nesting + 1);
        }

        if(process_header) {
            state.header_text = nullptr;

            QRegularExpression regex { "\\s+", QRegularExpression::DotMatchesEverythingOption };

            QString const header = header_text.replace(regex, " ");
            QString const anchor = QString("header-%1").arg(state.header_count);

            switch(element.tag) {
            case GUMBO_TAG_H1:
                outline.appendH1(header, anchor);
                break;
            case GUMBO_TAG_H2:
                outline.appendH2(header, anchor);
                break;
            case GUMBO_TAG_H3:
                outline.appendH3(header, anchor);
                break;
            case GUMBO_TAG_H4:
                // TODO: Support H4 headings
                break;
            case GUMBO_TAG_H5:
                // TODO: Support H5 headings
                break;
            case GUMBO_TAG_H6:
                // TODO: Support H6 headings
                break;
            default:
                break;
            }
            state.header_count += 1;
        }

        stream += "</" + QString::fromUtf8(gumbo_normalized_tagname(element.tag)) + ">";

        // qDebug() << "end node(" << gumbo_normalized_tagname(element.tag) << ")";

        break;
    }


    case GUMBO_NODE_TEXT:    // Text node.v will be a GumboText.
    case GUMBO_NODE_CDATA: { // CDATA node.v will be a GumboText.
        auto const & text = node.v.text;
        QString raw = QString::fromUtf8(text.text);

        if(state.header_text != nullptr) {
            (*state.header_text) += raw;
        }

        stream += raw.toHtmlEscaped();
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

    outline.beginBuild();

    // Find page title
    {
        const char* title = find_title(gumbo_output->root);
        if(title != nullptr) {
            page_title = QString::fromUtf8(title);
        }
    }

    QString document_text = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head>";
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
                document_text,
                root_url,
                outline,
                nullptr,
                0,
            };
            renderRecursive(state, *body);
        }
    }
    document_text += "</html>";

    outline.endBuild();

    auto document = std::make_unique<QTextDocument>();
    renderhelpers::setPageMargins(document.get(), style.margin_h, style.margin_v);
    document->setIndentWidth(style.indent_size);

    document->setDefaultFont(style.standard_font);
    document->setDefaultStyleSheet(style.toStyleSheet());
    document->setHtml(document_text);

    return document;
}
