
HEADERS += \
  $$PWD/src/buffer.h \
  $$PWD/src/chunk.h \
  $$PWD/src/cmark.h \
  $$PWD/src/cmark_export.h \
  $$PWD/src/cmark_version.h \
  $$PWD/src/config.h \
  $$PWD/src/cmark_ctype.h \
  $$PWD/src/config.h \
  $$PWD/src/houdini.h \
  $$PWD/src/inlines.h \
  $$PWD/src/iterator.h \
  $$PWD/src/node.h \
  $$PWD/src/parser.h \
  $$PWD/src/references.h \
  $$PWD/src/render.h \
  $$PWD/src/scanners.h \
  $$PWD/src/utf8.h

SOURCES += \
  $$PWD/src/blocks.c \
  $$PWD/src/buffer.c \
  $$PWD/src/cmark.c \
  $$PWD/src/cmark_ctype.c \
  $$PWD/src/commonmark.c \
  $$PWD/src/houdini_href_e.c \
  $$PWD/src/houdini_html_e.c \
  $$PWD/src/houdini_html_u.c \
  $$PWD/src/inlines.c \
  $$PWD/src/iterator.c \
  $$PWD/src/node.c \
  $$PWD/src/references.c \
  $$PWD/src/render.c \
  $$PWD/src/scanners.c \
  $$PWD/src/utf8.c \
  $$PWD/src/html.c \
  $$PWD/src/xml.c

INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src

# exluded modules:
# $$PWD/src/latex.c \
# $$PWD/src/man.c \
# -- reserved for HTML :)
