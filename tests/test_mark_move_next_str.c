#include "test.h"

MAIN("hi\nana\nbanana",
    mark_move_beginning(cur);

    mark_move_next_str(cur, "ana", strlen("ana"));
    ASSERT("line1", buf->first_line->next, cur->bline);
    ASSERT("col1", 0, cur->col);

    mark_move_next_str(cur, "ana", strlen("ana"));
    ASSERT("line1again", buf->first_line->next, cur->bline);
    ASSERT("col1again", 0, cur->col);

    mark_move_next_str_nudge(cur, "ana", strlen("ana"));
    ASSERT("line2", buf->first_line->next->next, cur->bline);
    ASSERT("col2", 1, cur->col);

    mark_move_next_str_nudge(cur, "ana", strlen("ana"));
    ASSERT("line3", buf->first_line->next->next, cur->bline);
    ASSERT("col3", 3, cur->col);
)
