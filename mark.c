#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "mledit.h"

typedef char* (*mark_find_match_fn)(char* haystack, size_t haystack_len, void* u1, void* u2, size_t* ret_match_len);
static int mark_find_match(mark_t* self, mark_find_match_fn matchfn, void* u1, void* u2, int reverse, bline_t** ret_line, size_t* ret_col);
static char* mark_find_match_prev(char* haystack, size_t haystack_len, mark_find_match_fn matchfn, void* u1, void* u2);
static int mark_find_re(mark_t* self, char* re, size_t re_len, int reverse, bline_t** ret_line, size_t* ret_col);
static char* mark_find_next_str_matchfn(char* haystack, size_t haystack_len, void* needle, void* needle_len, size_t* ret_needle_len);
static char* mark_find_prev_str_matchfn(char* haystack, size_t haystack_len, void* needle, void* needle_len, size_t* ret_needle_len);
static char* mark_find_next_cre_matchfn(char* haystack, size_t haystack_len, void* cre, void* unused, size_t* ret_needle_len);
static char* mark_find_prev_cre_matchfn(char* haystack, size_t haystack_len, void* cre, void* unused, size_t* ret_needle_len);

// Return a clone (same position) of an existing mark
mark_t* mark_clone(mark_t* self) {
    mark_t* new_mark;
    new_mark = buffer_add_mark(self->bline->buffer, self->bline, self->col);
    return new_mark;
}

// Insert data before mark
int mark_insert_before(mark_t* self, char* data, size_t data_len) {
    int rc;
    size_t num_chars;
    if ((rc = bline_insert(self->bline, self->col, data, data_len, &num_chars)) == MLEDIT_OK) {
        rc = mark_move_by(self, ((ssize_t)num_chars) * -1);
    }
    return rc;
}

// Insert data after mark
int mark_insert_after(mark_t* self, char* data, size_t data_len) {
    return bline_insert(self->bline, self->col, data, data_len, NULL);
}

// Delete data after mark
int mark_delete_after(mark_t* self, size_t num_chars) {
    return bline_delete(self->bline, self->col, num_chars);
}

// Delete data before mark
int mark_delete_before(mark_t* self, size_t num_chars) {
    int rc;
    if ((rc = mark_move_by(self, -1 * (ssize_t)num_chars)) == MLEDIT_OK) {
        rc = mark_delete_after(self, num_chars);
    }
    return rc;
}

// Move mark by a character delta
int mark_move_by(mark_t* self, ssize_t char_delta) {
    size_t offset;
    buffer_get_offset(self->bline->buffer, self->bline, self->col, &offset);
    return mark_move_offset(
        self,
        (size_t)MLEDIT_MIN(self->bline->buffer->char_count,
            MLEDIT_MAX(0, (ssize_t)offset + char_delta)
        )
    );
}

// Move mark by line delta
int mark_move_vert(mark_t* self, ssize_t line_delta) {
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
        return MLEDIT_OK;
    }
    MLEDIT_MARK_MOVE(self, cur_line, self->target_col, 0);
    return MLEDIT_OK;
}

// Move mark to beginning of line
int mark_move_bol(mark_t* self) {
    MLEDIT_MARK_SET_COL(self, 0, 1);
    return MLEDIT_OK;
}

// Move mark to end of line
int mark_move_eol(mark_t* self) {
    MLEDIT_MARK_SET_COL(self, self->bline->char_count, 1);
    return MLEDIT_OK;
}

// Move mark to a column on the current line
int mark_move_col(mark_t* self, size_t col) {
    MLEDIT_MARK_SET_COL(self, col, 1);
    return MLEDIT_OK;
}

// Move mark to beginning of buffer
int mark_move_beginning(mark_t* self) {
    MLEDIT_MARK_MOVE(self, self->bline->buffer->first_line, 0, 1);
    return MLEDIT_OK;
}

// Move mark to end of buffer
int mark_move_end(mark_t* self) {
    MLEDIT_MARK_MOVE(self, self->bline->buffer->last_line, self->bline->buffer->last_line->char_count, 1);
    return MLEDIT_OK;
}

// Move mark to a particular offset
int mark_move_offset(mark_t* self, size_t offset) {
    bline_t* dest_line;
    size_t dest_col;
    buffer_get_bline_col(self->bline->buffer, offset, &dest_line, &dest_col);
    MLEDIT_MARK_MOVE(self, dest_line, dest_col, 1);
    return MLEDIT_OK;
}

// Find next occurrence of string from mark
int mark_find_next_str(mark_t* self, char* str, size_t str_len, bline_t** ret_line, size_t* ret_col) {
    return mark_find_match(self, mark_find_next_str_matchfn, (void*)str, (void*)&str_len, 0, ret_line, ret_col);
}

// Find prev occurrence of string from mark
int mark_find_prev_str(mark_t* self, char* str, size_t str_len, bline_t** ret_line, size_t* ret_col) {
    return mark_find_match(self, mark_find_prev_str_matchfn, (void*)str, (void*)&str_len, 1, ret_line, ret_col);
}

// Find next occurence of regex from mark
int mark_find_next_cre(mark_t* self, pcre* cre, bline_t** ret_line, size_t* ret_col) {
    return mark_find_match(self, mark_find_next_cre_matchfn, (void*)cre, NULL, 0, ret_line, ret_col);
}

// Find prev occurence of regex from mark
int mark_find_prev_cre(mark_t* self, pcre* cre, bline_t** ret_line, size_t* ret_col) {
    return mark_find_match(self, mark_find_prev_cre_matchfn, (void*)cre, NULL, 0, ret_line, ret_col);
}

// Find next occurence of uncompiled regex str from mark
int mark_find_next_re(mark_t* self, char* re, size_t re_len, bline_t** ret_line, size_t* ret_col) {
    return mark_find_re(self, re, re_len, 0, ret_line, ret_col);
}

// Find prev occurence of uncompiled regex str from mark
int mark_find_prev_re(mark_t* self, char* re, size_t re_len, bline_t** ret_line, size_t* ret_col) {
    return mark_find_re(self, re, re_len, 1, ret_line, ret_col);
}

#define MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(mark, findfn, ...) \
    int rc; \
    bline_t* line; \
    size_t col; \
    if ((rc = (findfn)((mark), __VA_ARGS__, &line, &col)) == MLEDIT_OK) { \
        MLEDIT_MARK_MOVE((mark), line, col, 1); \
        return MLEDIT_OK; \
    } \
    return rc;

int mark_move_next_str(mark_t* self, char* str, size_t str_len) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_str, str, str_len)
}

int mark_move_prev_str(mark_t* self, char* str, size_t str_len) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_str, str, str_len)
}

int mark_move_next_cre(mark_t* self, pcre* cre) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_cre, cre)
}

int mark_move_prev_cre(mark_t* self, pcre* cre) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_cre, cre)
}

int mark_move_next_re(mark_t* self, char* re, size_t re_len) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_next_re, re, re_len)
}

int mark_move_prev_re(mark_t* self, char* re, size_t re_len) {
    MLEDIT_MARK_IMPLEMENT_MOVE_VIA_FIND(self, mark_find_prev_re, re, re_len)
}

int mark_find_bracket_pair(mark_t* self, char bracket, bline_t** ret_line, size_t* ret_col); // TODO
int mark_move_bracket_pair(mark_t* self, char bracket); // TODO
int mark_delete_between_mark(mark_t* self, mark_t* other); // TODO
int mark_get_between_mark(mark_t* self, mark_t* other, char** ret_str, size_t* ret_str_len); // TODO
int mark_swap_with_mark(mark_t* self, mark_t* other); // TODO

// Free a mark
int mark_destroy(mark_t* self) {
    free(self);
    return MLEDIT_OK;
}

// Find first occurrence of match according to matchfn. Search backwards if
// reverse is truthy.
static int mark_find_match(mark_t* self, mark_find_match_fn matchfn, void* u1, void* u2, int reverse, bline_t** ret_line, size_t* ret_col) {
    bline_t* search_line;
    char* match;
    size_t search_start;
    size_t search_len;
    size_t match_col;
    search_line = self->bline;
    if (reverse) {
        search_start = 0;
        search_len = self->col < search_line->char_count ? search_line->char_indexes[self->col] : search_line->data_len;
    } else {
        search_start = self->col < search_line->char_count ? search_line->char_indexes[self->col] : search_line->data_len;
        search_len = search_line->data_len - search_start;
    }
    while (search_line) {
        if (search_line->char_count > 0 && search_len > 0) {
            match = matchfn(search_line->data + search_start, search_len, u1, u2, NULL);
            if (match != NULL) {
                bline_get_col(search_line, (size_t)(match - search_line->data), &match_col);
                *ret_line = search_line;
                *ret_col = match_col;
                return MLEDIT_OK;
            }
        }
        search_line = reverse ? search_line->prev : search_line->next;
        if (search_line) {
            search_start = 0;
            search_len = search_line->data_len;
        }
    }
    *ret_line = NULL;
    return MLEDIT_ERR;
}

// Return the last occurrence of a match given a forward-searching matchfn
static char* mark_find_match_prev(char* haystack, size_t haystack_len, mark_find_match_fn matchfn, void* u1, void* u2) {
    char* match;
    char* last_match;
    size_t next_offset;
    size_t match_len;
    last_match = NULL;
    while (1) {
        match = matchfn(haystack, haystack_len, u1, u2, &match_len);
        if (match == NULL) {
            return last_match;
        }
        next_offset = (size_t)(match - haystack) + match_len;
        if (next_offset + match_len > haystack_len) {
            return match;
        }
        haystack = match + match_len;
        haystack_len -= next_offset;
        last_match = match;
    }
}

// Find uncompiled regex from mark. Search backwards if reverse is truthy.
static int mark_find_re(mark_t* self, char* re, size_t re_len, int reverse, bline_t** ret_line, size_t* ret_col) {
    int rc;
    char* regex;
    pcre* cre;
    const char *error;
    int erroffset;
    regex = malloc(re_len + 1);
    snprintf(regex, re_len, "%s", re);
    cre = pcre_compile((const char*)regex, PCRE_NO_AUTO_CAPTURE, &error, &erroffset, NULL); // TODO utf8
    if (cre == NULL) {
        // TODO log error
        free(regex);
        return MLEDIT_ERR;
    }
    if (reverse) {
        rc = mark_find_prev_cre(self, cre, ret_line, ret_col);
    } else {
        rc = mark_find_next_cre(self, cre, ret_line, ret_col);
    }
    pcre_free(cre);
    return rc;
}

static char* mark_find_next_str_matchfn(char* haystack, size_t haystack_len, void* needle, void* needle_len, size_t* ret_needle_len) {
    if (ret_needle_len) *ret_needle_len = *((size_t*)needle_len);
    return memmem(haystack, haystack_len, needle, *((size_t*)needle_len));
}

static char* mark_find_prev_str_matchfn(char* haystack, size_t haystack_len, void* needle, void* needle_len, size_t* ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, mark_find_next_str_matchfn, needle, needle_len);
}

static char* mark_find_next_cre_matchfn(char* haystack, size_t haystack_len, void* cre, void* unused, size_t* ret_needle_len) {
    int rc;
    int substrs[3];
    if ((rc = pcre_exec((pcre*)cre, NULL, haystack, haystack_len, 0, 0, substrs, 3)) >= 0) {
        if (ret_needle_len) *ret_needle_len = (size_t)(substrs[1] - substrs[0]);
        return haystack + rc;
    }
    return NULL;
}

static char* mark_find_prev_cre_matchfn(char* haystack, size_t haystack_len, void* cre, void* unused, size_t* ret_needle_len) {
    return mark_find_match_prev(haystack, haystack_len, mark_find_next_cre_matchfn, cre, unused);
}
