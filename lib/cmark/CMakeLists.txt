cmake_minimum_required(VERSION 3.13)
project(cmark C)

set(sources
  src/blocks.c
  src/buffer.c
  src/cmark.c
  src/cmark_ctype.c
  src/commonmark.c
  src/houdini_href_e.c
  src/houdini_html_e.c
  src/houdini_html_u.c
  src/inlines.c
  src/iterator.c
  src/node.c
  src/references.c
  src/render.c
  src/scanners.c
  src/utf8.c
  src/html.c
  src/xml.c)

add_library(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC src)
