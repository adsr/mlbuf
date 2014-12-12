#define ASSERT(testname, expected, observed) do { \
    if ((expected) == (observed)) { \
        printf("  \x1b[32mOK \x1b[0m %s\n", (testname)); \
    } else { \
        printf("  \x1b[31mERR\x1b[0m %s expected=%lu observed=%lu\n", (testname), (size_t)(expected), (size_t)(observed)); \
        exit(EXIT_FAILURE); \
    } \
} while(0);
