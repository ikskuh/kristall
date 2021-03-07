#DISTFILES += \
#  $$PWD/tag.in

INCLUDEPATH += $$quote($$PWD/include)

HEADERS += \
  $$PWD/attribute.h \
  $$PWD/char_ref.h \
  $$PWD/char_ref.rl \
  $$PWD/error.h \
  $$PWD/gumbo-utf8.h \
  $$PWD/include/gumbo.h \
  $$PWD/insertion_mode.h \
  $$PWD/parser.h \
  $$PWD/string_buffer.h \
  $$PWD/string_piece.h \
  $$PWD/include/tag_enum.h \
  $$PWD/tag_gperf.h \
  $$PWD/tag_sizes.h \
  $$PWD/tag_strings.h \
  $$PWD/token_type.h \
  $$PWD/tokenizer.h \
  $$PWD/tokenizer_states.h \
  $$PWD/util.h \
  $$PWD/vector.h

SOURCES += \
  $$PWD/attribute.c \
  $$PWD/char_ref.c \
  $$PWD/error.c \
  $$PWD/gumbo-utf8.c \
  $$PWD/parser.c \
  $$PWD/string_buffer.c \
  $$PWD/string_piece.c \
  $$PWD/tag.c \
  $$PWD/tokenizer.c \
  $$PWD/util.c \
  $$PWD/vector.c
