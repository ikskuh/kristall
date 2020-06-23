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

  block_quote_format.setIndent(1);
  block_quote_format.setBackground(themed_style.blockquote_color);
}
