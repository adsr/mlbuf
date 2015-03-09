#include <string.h>
#include "mlbuf.h"

void* recalloc(void* ptr, size_t orig_num, size_t new_num, size_t el_size) {
    void* newptr;
    newptr = realloc(ptr, new_num * el_size);
    if (!newptr) return NULL;
    if (new_num > orig_num) {
        memset(newptr + (orig_num * el_size), 0, (new_num - orig_num) * el_size);
    }
    return newptr;
}
