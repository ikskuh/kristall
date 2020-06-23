#ifndef TEXTSTYLEINSTANCE_HPP
#define TEXTSTYLEINSTANCE_HPP

#include <QTextCharFormat>
#include <QTextBlockFormat>

#include "documentstyle.hpp"

struct TextStyleInstance
{
    QTextCharFormat preformatted;
    QTextCharFormat standard;
    QTextCharFormat standard_link;
    QTextCharFormat external_link;
    QTextCharFormat cross_protocol_link;
    QTextCharFormat standard_h1;
    QTextCharFormat standard_h2;
    QTextCharFormat standard_h3;

    QTextBlockFormat standard_format;
    QTextBlockFormat preformatted_format;
    QTextBlockFormat block_quote_format;

    explicit TextStyleInstance(DocumentStyle const & style);
};

#endif // TEXTSTYLEINSTANCE_HPP
