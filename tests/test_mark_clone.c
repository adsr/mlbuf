#include "test.h"

MAIN("hello\nworld",
    mark_t* other;
    other = mark_clone(cur);
    ASSERT("neq", 1, other != cur ? 1 : 0);
    ASSERT("next", 1, other == cur->next ? 1 : 0);
    ASSERT("line", cur->bline, other->bline);
    ASSERT("col", cur->col, other->col);
)
