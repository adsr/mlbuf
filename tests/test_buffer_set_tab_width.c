#include "test.h"

#define comma ,

MAIN("he\tllo\t\t",
    size_t i;
    size_t char_posz_4[8] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8};
    size_t char_posz_2a[8] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8};
    size_t char_posz_2b[9] = {0 comma  1 comma  2 comma  4 comma  5 comma  6 comma  7 comma  8 comma  10};

    buffer_set_tab_width(buf, 4);
    // [he  llo     ] // char_pos
    // [  t    tt   ] // tabs
    ASSERT("count4", 8, buf->first_line->char_count);
    ASSERT("width4", 12, buf->first_line->char_width);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("pos4", char_posz_4[i], buf->first_line->char_pos[i]);
    }

    buffer_set_tab_width(buf, 2);
    // [he  llo   ] // char_pos
    // [  t    tt ] // tabs
    ASSERT("count2a", 8, buf->first_line->char_count);
    ASSERT("width2a", 10, buf->first_line->char_width);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("pos2a", char_posz_2a[i], buf->first_line->char_pos[i]);
    }

    bline_insert(buf->first_line, 4, "\t", 1, NULL);
    // [he  l lo    ] // char_pos
    // [  t  t  t t ] // tabs
    ASSERT("count2b", 9, buf->first_line->char_count);
    ASSERT("width2b", 12, buf->first_line->char_width);
    for (i = 0; i < buf->first_line->char_count; i++) {
        ASSERT("pos2b", char_posz_2b[i], buf->first_line->char_pos[i]);
    }
)
