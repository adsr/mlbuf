#include "test.h"

MAIN("hello\nworld",
    uintmax_t hash;
    hash = buffer_hash(buf);
    ASSERT("yes", hash != 0 ? 1 : 0, 1);
)

