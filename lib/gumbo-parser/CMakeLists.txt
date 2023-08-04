cmake_minimum_required(VERSION 3.13)
project(gumbo-parser C)
set(sources
    attribute.c
    char_ref.c
    error.c
    gumbo-utf8.c
    parser.c
    string_buffer.c
    string_piece.c
    tag.c
    tokenizer.c
    util.c
    vector.c
)
add_library(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE .)
