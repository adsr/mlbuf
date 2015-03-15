#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "mlbuf.h"

typedef char* (*mark_find_match_fn)(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* u1, void* u2, bint_t* ret_match_len);
static int mark_find_match(mark_t* self, mark_find_match_fn matchfn, void* u1, void* u2, int reverse, bline_t** ret_line, bint_t* ret_col);
static int mark_find_re(mark_t* self, char* re, bint_t re_len, int reverse, bline_t** ret_line, bint_t* ret_col);
static char* mark_find_match_prev(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, mark_find_match_fn matchfn, void* u1, void* u2);
static char* mark_find_next_str_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* needle, void* needle_len, bint_t* ret_needle_len);
static char* mark_find_prev_str_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* needle, void* needle_len, bint_t* ret_needle_len);
static char* mark_find_next_cre_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* cre, void* unused, bint_t* ret_needle_len);
static char* mark_find_prev_cre_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* cre, void* unused, bint_t* ret_needle_len);

// Return a clone (same position) of an existing mark
mark_t* mark_clone(mark_t* self) {
    mark_t* new_mark;
    new_mark = buffer_add_mark(self->bline->buffer, self->bline, self->col);
    return new_mark;
}

// Insert data before mark
int mark_insert_before(mark_t* self, char* data, bint_t data_len) {
    return bline_insert(self->bline, self->col, data, data_len, NULL);
}

// Insert data after mark
int mark_insert_after(mark_t* self, char* data, bint_t data_len) {
    int rc;
    bint_t num_chars;
    if ((rc = bline_insert(self->bline, self->col, data, data_len, &num_chars)) == MLBUF_OK) {
        rc = mark_move_by(self, -1 * num_chars);
    }
    return rc;
}

// Delete data after mark
int mark_delete_after(mark_t* self, bint_t num_chars) {
    return bline_delete(self->bline, self->col, num_chars);
}

// Delete data before mark
int mark_delete_before(mark_t* self, bint_t num_chars) {
    int rc;
    if ((rc = mark_move_by(self, -1 * num_chars)) == MLBUF_OK) {
        rc = mark_delete_after(self, num_chars);
    }
    return rc;
}

// Move mark to line_index:col
int mark_move_to(mark_t* self, bint_t line_index, bint_t col) {
    bline_t* bline;
    buffer_get_bline(self->bline->buffer, line_index, &bline);
    MLBUF_MARK_MOVE(self, bline, col, 1);
    return MLBUF_OK;
}

// Move mark by a character delta
int mark_move_by(mark_t* self, bint_t char_delta) {
    bint_t offset;
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset);
    return mark_move_offset(
        self,
        MLBUF_MIN(self->bline->buffer->char_count,
            MLBUF_MAX(0, offset + char_delta)
        )
    );
}

// Get mark offset
int mark_get_offset(mark_t* self, bint_t* ret_offset) {
    return buffer_get_offset(self->bline->buffer, self->bline, self->col, ret_offset);
}

// Move mark by line delta
int mark_move_vert(mark_t* self, bint_t line_delta) {
    bline_t* cur_line;
    bline_t* tmp_line;
    cur_line = self->bline;
    while (line_delta != 0) {
        tmp_line = line_delta > 0 ? cur_line->next : cur_line->prev;
        if (!tmp_line) {
            break;
        }
        cur_line = tmp_line;
        line_delta = line_delta + (line_delta > 0 ? -1 : 1);
    }
    if (cur_line == self->bline) {
        return MLBUF_OK;
    }
    MLBUF_MARK_MOVE(self, cur_line, self->target_col, 0);
    return MLBUF_OK;
}

// Move mark to beginning of line
int mark_move_bol(mark_t* self) {
    MLBUF_MARK_SET_COL(self, 0, 1);
    return MLBUF_OK;
}

// Move mark to end of line
int mark_move_eol(mark_t* self) {
    MLBUF_MARK_SET_COL(self, self->bline->char_count, 1);
    return MLBUF_OK;
}

// Move mark to a column on the current line
int mark_move_col(mark_t* self, bint_t col) {
    MLBUF_MARK_SET_COL(self, col, 1);
    return MLBUF_OK;
}

// Move mark to beginning of buffer
int mark_move_beginning(mark_t* self) {
    MLBUF_MARK_MOVE(self, self->bline->buffer->first_line, 0, 1);
    return MLBUF_OK;
}

// Move mark to end of buffer
int mark_move_end(mark_t* self) {
    MLBUF_MARK_MOVE(self, self->bline->buffer->last_line, self->bline->buffer->last_line->char_count, 1);
    return MLBUF_OK;
}

// Move mark to a particular offset
int mark_move_offset(mark_t* self, bint_t offset) {
    bline_t* dest_line;
    bint_t dest_col;
    buffer_get_bline_col(self->bline->buffer, offset, &dest_line, &dest_col);
    MLBUF_MARK_MOVE(self, dest_line, dest_col, 1);
    return MLBUF_OK;
}

// Find next occurrence of string from mark
int mark_find_next_str(mark_t* self, char* str, bint_t str_len, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_match(self, mark_find_next_str_matchfn, (void*)str, (void*)&str_len, 0, ret_line, ret_col);
}

// Find prev occurrence of string from mark
int mark_find_prev_str(mark_t* self, char* str, bint_t str_len, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_match(self, mark_find_prev_str_matchfn, (void*)str, (void*)&str_len, 1, ret_line, ret_col);
}

// Find next occurence of regex from mark
int mark_find_next_cre(mark_t* self, pcre* cre, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_match(self, mark_find_next_cre_matchfn, (void*)cre, NULL, 0, ret_line, ret_col);
}

// Find prev occurence of regex from mark
int mark_find_prev_cre(mark_t* self, pcre* cre, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_match(self, mark_find_prev_cre_matchfn, (void*)cre, NULL, 1, ret_line, ret_col);
}

// Find next occurence of uncompiled regex str from mark
int mark_find_next_re(mark_t* self, char* re, bint_t re_len, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_re(self, re, re_len, 0, ret_line, ret_col);
}

// Find prev occurence of uncompiled regex str from mark
int mark_find_prev_re(mark_t* self, char* re, bint_t re_len, bline_t** ret_line, bint_t* ret_col) {
    return mark_find_re(self, re, re_len, 1, ret_line, ret_col);
}

// Return 1 if self is before other, otherwise return 0
int mark_is_lt(mark_t* self, mark_t* other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col < other->col ? 1 : 0;
    } else if (self->bline->line_index < other->bline->line_index) {
        return 1;
    }
    return 0;
}

// Return 1 if self is past other, otherwise return 0
int mark_is_gt(mark_t* self, mark_t* other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col > other->col ? 1 : 0;
    } else if (self->bline->line_index > other->bline->line_index) {
        return 1;
    }
    return 0;
}

// Return 1 if self is at same position as other, otherwise return 0
int mark_is_eq(mark_t* self, mark_t* other) {
    if (self->bline->line_index == other->bline->line_index) {
        return self->col == other->col ? 1 : 0;
    }
    return 0;
}

// Find the matching bracket character under the mark, examining no more than
// max_chars.
int mark_find_bracket_pair(mark_t* self, bint_t max_chars, bline_t** ret_line, bint_t* ret_col) {
    char brkt;
    char targ;
    char cur;
    int dir;
    int i;
    int nest;
    bint_t col;
    bint_t nchars;
    bline_t* cur_line;
    static char pairs[8] = {
        '[', ']',
        '(', ')',
        '{', '}',
        '<', '>'
    };
    // If we're at eol, there's nothing to match
    if (self->col >= self->bline->char_count) {
        return MLBUF_ERR;
    }
    // Set brkt to char under mark
    brkt = *(self->bline->data + self->bline->chars[self->col].index);
    // Find targ matching bracket char
    targ = 0;
    for (i = 0; i < 8; i++) {
        if (pairs[i] == brkt) {
            if (i % 2 == 0) {
                targ = pairs[i + 1];
                dir = 1;
            } else {
                targ = pairs[i - 1];
                dir = -1;
            }
            break;
        }
    }
    // If targ is not set, brkt was not a bracket char
    if (!targ) {
        return MLBUF_ERR;
    }
    // Now look for targ, keeping track of nesting
    // Break if we look at more than max_chars
    nest = -1;
    cur_line = self->bline;
    col = self->col;
    nchars = 0;
    while (cur_line) {
        for (; col >= 0 && col < cur_line->char_count; col += dir) {
            cur = *(cur_line->data + cur_line->chars[col].index);
            if (cur == targ) {
                if (nest == 0) {
                    // Match!
                    *ret_line = cur_line;
                    *ret_col = col;
                    return MLBUF_OK;
                } else {
                    nest -= 1;
                }
            } else if (cur == brkt) {
                nest += 1;
            }
            nchars += 1;
            if (nchars >= max_chars) {
                return MLBUF_ERR;
            }
        }
        if (dir > 0) {
            cur_line = cur_line->next;
            if (cur_line) col = 0;
        } else {
            cur_line = cur_line->prev;
            if (cur_line) col = MLBUF_MAX(1, cur_line->char_count) - 1;
        }
    }
    // If we got here, targ was not found, or nesting was off
    return MLBUF_ERR;
}

// Delete data between self and other
int mark_delete_between_mark(mark_t* self, mark_t* other) {
    bint_t offset_a;
    bint_t offset_b;
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset_a);
    buffer_get_offset(other->bline->buffer, other->bline, other->col, &offset_b);
    if (offset_a == offset_b) {
        return MLBUF_OK;
    } else if (offset_a > offset_b) {
        return buffer_delete(self->bline->buffer, offset_b, offset_a - offset_b);
    }
    return buffer_delete(self->bline->buffer, offset_a, offset_b - offset_a);
}

// Return data between self and other
int mark_get_between_mark(mark_t* self, mark_t* other, char** ret_str, bint_t* ret_str_len) {
    bint_t ig;
    if (mark_is_gt(self, other)) {
        return buffer_substr(
            self->bline->buffer,
            other->bline, other->col,
            self->bline, self->col,
            ret_str, ret_str_len, &ig
        );
    } else if (mark_is_gt(other, self)) {
        return buffer_substr(
            self->bline->buffer,
            self->bline, self->col,
            other->bline, other->col,
            ret_str, ret_str_len, &ig
        );
    }
    *ret_str = strdup("");
    *ret_str_len = 0;
    return MLBUF_OK;
}

// Swap positions of self and other
int mark_swap_with_mark(mark_t* self, mark_t* other) {
    bline_t* tmp_bline;
    bint_t tmp_col;
    tmp_bline = other->bline;
    tmp_col = other->col;
    other->bline = self->bline;
    other->col = self->col;
    self->bline = tmp_bline;
    self->col = tmp_col;
    return MLBUF_OK;
}

// Destroy a mark
int mark_destroy(mark_t* self) {
    return buffer_destroy_mark(self->bline->buffer, self);
}

#define MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(mark, findfn, ...) \
    int rc; \
    bline_t* line; \
    bint_t col; \
    if ((rc = (findfn)((mark), __VA_ARGS__, &line, &col)) == MLBUF_OK) { \
        MLBUF_MARK_MOVE((mark), line, col, 1); \
        return MLBUF_OK; \
    } \
    return rc;

int mark_move_next_str(mark_t* self, char* str, bint_t str_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_str, str, str_len)
}

int mark_move_prev_str(mark_t* self, char* str, bint_t str_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_str, str, str_len)
}

int mark_move_next_cre(mark_t* self, pcre* cre) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_cre, cre)
}

int mark_move_prev_cre(mark_t* self, pcre* cre) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_cre, cre)
}

int mark_move_next_re(mark_t* self, char* re, bint_t re_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_re, re, re_len)
}

int mark_move_prev_re(mark_t* self, char* re, bint_t re_len) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_re, re, re_len)
}

int mark_move_bracket_pair(mark_t* self, bint_t max_chars) {
    MLBUF_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_bracket_pair, max_chars);
}

// Find first occurrence of match according to matchfn. Search backwards if
// reverse is truthy.
static int mark_find_match(mark_t* self, mark_find_match_fn matchfn, void* u1, void* u2, int reverse, bline_t** ret_line, bint_t* ret_col) {
    bline_t* search_line;
    char* match;
    bint_t look_offset;
    bint_t match_col;
    bint_t max_offset;
    search_line = self->bline;
    if (reverse) {
        look_offset = 0;
        max_offset = self->col < 1 ? 0 : search_line->chars[self->col - 1].index;
    } else {
        look_offset = self->col + 1 < search_line->char_count ? search_line->chars[self->col + 1].index : search_line->data_len;
        max_offset = search_line->data_len - 1;
    }
    while (search_line) {
        match = matchfn(search_line->data, search_line->data_len, look_offset, max_offset, u1, u2, NULL);
        if (match != NULL) {
            bline_get_col(search_line, (bint_t)(match - search_line->data), &match_col);
            *ret_line = search_line;
            *ret_col = match_col;
            return MLBUF_OK;
        }
        search_line = reverse ? search_line->prev : search_line->next;
        if (search_line) {
            look_offset = 0;
            max_offset = search_line->data_len - 1;
        }
    }
    *ret_line = NULL;
    return MLBUF_ERR;
}

// Return the last occurrence of a match given a forward-searching matchfn
static char* mark_find_match_prev(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, mark_find_match_fn matchfn, void* u1, void* u2) {
    char* match;
    char* last_match;
    bint_t match_len;
    last_match = NULL;
    while (1) {
        match = matchfn(haystack, haystack_len, look_offset, max_offset, u1, u2, &match_len);
        if (match == NULL) {
            return last_match;
        }
        if (match - haystack > max_offset) {
            return last_match;
        }
        // Override match_len to 1. Reasoning: If we have a haystack like
        // 'banana' and our re is 'ana', using the actual match_len for the
        // next search offset would skip the 2nd 'ana' match.
        match_len = 1;
        look_offset = (bint_t)(match - haystack) + match_len;
        if (look_offset + match_len > haystack_len) {
            return match;
        }
        last_match = match;
    }
}

// Find uncompiled regex from mark. Search backwards if reverse is truthy.
static int mark_find_re(mark_t* self, char* re, bint_t re_len, int reverse, bline_t** ret_line, bint_t* ret_col) {
    int rc;
    char* regex;
    pcre* cre;
    const char *error;
    int erroffset;
    MLBUF_MAKE_GT_EQ0(re_len);
    regex = malloc(re_len + 1);
    snprintf(regex, re_len + 1, "%s", re);
    cre = pcre_compile((const char*)regex, PCRE_NO_AUTO_CAPTURE, &error, &erroffset, NULL); // TODO utf8
    if (cre == NULL) {
        // TODO log error
        free(regex);
        return MLBUF_ERR;
    }
    if (reverse) {
        rc = mark_find_prev_cre(self, cre, ret_line, ret_col);
    } else {
        rc = mark_find_next_cre(self, cre, ret_line, ret_col);
    }
    pcre_free(cre);
    return rc;
}

static char* mark_find_next_str_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* needle, void* needle_len, bint_t* ret_needle_len) {
    if (ret_needle_len) *ret_needle_len = *((bint_t*)needle_len);
    if (look_offset >= haystack_len) return NULL;
    return memmem(haystack + look_offset, haystack_len - look_offset, needle, *((bint_t*)needle_len));
}

static char* mark_find_prev_str_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* needle, void* needle_len, bint_t* ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, look_offset, max_offset, mark_find_next_str_matchfn, needle, needle_len);
}

static char* mark_find_next_cre_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* cre, void* unused, bint_t* ret_needle_len) {
    int rc;
    int substrs[3];
    MLBUF_INIT_PCRE_EXTRA(pcre_extra);
    if ((rc = pcre_exec((pcre*)cre, &pcre_extra, haystack, haystack_len, look_offset, 0, substrs, 3)) >= 0) {
        if (ret_needle_len) *ret_needle_len = (bint_t)(substrs[1] - substrs[0]);
        return haystack + substrs[0];
    }
    return NULL;
}

static char* mark_find_prev_cre_matchfn(char* haystack, bint_t haystack_len, bint_t look_offset, bint_t max_offset, void* cre, void* unused, bint_t* ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, look_offset, max_offset, mark_find_next_cre_matchfn, cre, unused);
}
