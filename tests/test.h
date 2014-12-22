#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../mlbuf.h"

#define MAIN(str, body) \
int main(int argc, char **argv) { \
    buffer_t* buf; \
    mark_t* cur; \
    buf = buffer_new(); \
    cur = buffer_add_mark(buf, NULL, 0); \
    if (cur) { } \
    buffer_insert(buf, 0, (char*)str, (size_t)strlen(str), NULL); \
    body \
    buffer_destroy(buf); \
    return EXIT_SUCCESS; \
}

#define ASSERT(testname, expected, observed) do { \
    if ((expected) == (observed)) { \
        printf("  \x1b[32mOK \x1b[0m %s\n", (testname)); \
    } else { \
        printf("  \x1b[31mERR\x1b[0m %s expected=%lu observed=%lu\n", (testname), (size_t)(expected), (size_t)(observed)); \
        exit(EXIT_FAILURE); \
    } \
} while(0);
