#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../mledit.h"
#include "assert.h"

int main(int argc, char** argv) {
    buffer_t* buf;
    mark_t* mark;
    size_t num_chars;
    buf = buffer_new();
    buffer_insert(buf, 0, (char*)"hello", 13, &num_chars); // hello
    mark = buffer_add_mark(buf, buf->first_line, 3); // hel|lo
    ASSERT("mark_line", 1, buf->first_line == mark->bline ? 1 : 0);
    ASSERT("mark_col", 3, mark->col);
    ASSERT("mark_target_col", 3, mark->col);
    buffer_insert(buf, 0, (char*)"x", 1, &num_chars); // xhel|lo
    ASSERT("mark_pos_1", 4, mark->col);
    buffer_insert(buf, 4, (char*)"\n", 1, &num_chars); // xhel\n|lo
    ASSERT("mark_pos_2_line_a", 1, mark->bline == buf->last_line ? 1 : 0)
    ASSERT("mark_pos_2_line_b", 1, mark->bline == buf->first_line->next ? 1 : 0)
    ASSERT("mark_pos_2_col", 0, mark->col);
    buffer_insert(buf, 7, (char*)"pe", 2, &num_chars); // xhel\n|lope
    ASSERT("mark_pos_3_col", 0, mark->col);
    ASSERT("mark_bline_data", 0, strncmp("lope", mark->bline->data, 4));
    buffer_destroy(buf);
    exit(EXIT_SUCCESS);
}
