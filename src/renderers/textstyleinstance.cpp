#include "textstyleinstance.hpp"

TextStyleInstance::TextStyleInstance(DocumentStyle const & themed_style)
{
  preformatted.setFont(themed_style.preformatted_font);
  preformatted.setForeground(themed_style.preformatted_color);

  standard.setFont(themed_style.standard_font);
  standard.setForeground(themed_style.standard_color);

  standard_link.setFont(themed_style.standard_font);
  standard_link.setForeground(QBrush(themed_style.internal_link_color));

  external_link.setFont(themed_style.standard_font);
  external_link.setForeground(QBrush(themed_style.external_link_color));

  cross_protocol_link.setFont(themed_style.standard_font);
  cross_protocol_link.setForeground(QBrush(themed_style.cross_scheme_link_color));

  standard_h1.setFont(themed_style.h1_font);
  standard_h1.setForeground(QBrush(themed_style.h1_color));

  standard_h2.setFont(themed_style.h2_font);
  standard_h2.setForeground(QBrush(themed_style.h2_color));

  standard_h3.setFont(themed_style.h3_font);
  standard_h3.setForeground(QBrush(themed_style.h3_color));

  preformatted_format.setNonBreakableLines(true);

  block_quote_format.setIndent(themed_style.indent_bq);
  block_quote_format.setBackground(themed_style.blockquote_color);

  // Other alignments
  auto align = themed_style.justify_text ? Qt::AlignJustify : Qt::AlignLeft;

  standard_format.setAlignment(align);
  standard_format.setLineHeight(themed_style.line_height_p,
    QTextBlockFormat::LineDistanceHeight);
  standard_format.setIndent(themed_style.indent_p);

  link_format.setLineHeight(themed_style.line_height_p,
    QTextBlockFormat::LineDistanceHeight);

  block_quote_format.setAlignment(align);
  block_quote_format.setLineHeight(themed_style.line_height_p,
    QTextBlockFormat::LineDistanceHeight);

  list_format.setStyle(QTextListFormat::ListDisc);
  list_format.setIndent(themed_style.indent_l);

  preformatted_format.setIndent(themed_style.indent_p);

  heading_format.setLineHeight(themed_style.line_height_h,
    QTextBlockFormat::LineDistanceHeight);
  heading_format.setIndent(themed_style.indent_h);
}
