#include "mlbuf.h"

// Insert data on a line
int bline_insert(bline_t* self, bint_t col, char* data, bint_t data_len, bint_t* ret_num_chars) {
    bint_t offset;
    buffer_get_offset(self->buffer, self, col, &offset);
    return buffer_insert(self->buffer, offset, data, data_len, ret_num_chars);
}

// Delete data from a line
int bline_delete(bline_t* self, bint_t col, bint_t num_chars) {
    bint_t offset;
    buffer_get_offset(self->buffer, self, col, &offset);
    return buffer_delete(self->buffer, offset, num_chars);
}

// Return a col given a byte index
int bline_get_col(bline_t* self, bint_t index, bint_t* ret_col) {
    bint_t col;
    MLBUF_MAKE_GT_EQ0(index);
    if (index == 0 || self->char_count == 0) {
        *ret_col = 0;
        return MLBUF_OK;
    } else if (index >= self->data_len) {
        *ret_col = self->char_count;
        return MLBUF_OK;
    }
    for (col = 1; col < self->char_count; col++) {
        if (self->char_indexes[col] > index) {
            *ret_col = col - 1;
            return MLBUF_OK;
        } else if (self->char_indexes[col] == index) {
            *ret_col = col;
            return MLBUF_OK;
        }
    }
    *ret_col = self->char_count - (index < self->data_len ? 1 : 0);
    return MLBUF_OK;
}
