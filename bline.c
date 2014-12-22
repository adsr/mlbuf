#include "mlbuf.h"

// Insert data on a line
int bline_insert(bline_t* self, size_t col, char* data, size_t data_len, size_t* ret_num_chars) {
    size_t offset;
    buffer_get_offset(self->buffer, self, col, &offset);
    return buffer_insert(self->buffer, offset, data, data_len, ret_num_chars);
}

// Delete data from a line
int bline_delete(bline_t* self, size_t col, size_t num_chars) {
    size_t offset;
    buffer_get_offset(self->buffer, self, col, &offset);
    return buffer_delete(self->buffer, offset, num_chars);
}

// Return a col given a byte index
int bline_get_col(bline_t* self, size_t index, size_t* ret_col) {
    size_t col;
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
