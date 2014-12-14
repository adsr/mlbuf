#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../mledit.h"
#include "assert.h"

int main(int argc, char** argv) {
    buffer_t* buf;
    size_t num_chars;
    buf = buffer_new();
    ASSERT("byte_count_a", 0, buf->byte_count);
    ASSERT("char_count_a", 0, buf->char_count);
    ASSERT("line_count_a", 1, buf->line_count);
    buffer_insert(buf, 0, (char*)"line 1\nline 2", 13, &num_chars);
    ASSERT("num_chars", 13, num_chars);
    ASSERT("byte_count_b", 13, buf->byte_count);
    ASSERT("char_count_b", 13, buf->char_count);
    ASSERT("line_count_b", 2, buf->line_count);
    ASSERT("data_L1_b", 0, strncmp(buf->first_line->data, "line 1", 6));
    ASSERT("data_L2_b", 0, strncmp(buf->first_line->next->data, "line 2", 6));
    buffer_delete(buf, 6, 1);
    ASSERT("byte_count_c", 12, buf->byte_count);
    ASSERT("char_count_c", 12, buf->char_count);
    ASSERT("line_count_c", 1, buf->line_count);
    ASSERT("data_L1_c", 0, strncmp(buf->first_line->data, "line 1line 2", 12));
    buffer_destroy(buf);
    exit(EXIT_SUCCESS);
}
