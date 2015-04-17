#ifndef __MLBUF_H
#define __MLBUF_H

#include <stdio.h>
#include <stdint.h>
#include <pcre.h>
#include "utlist.h"

// Typedefs
typedef struct buffer_s buffer_t; // A buffer of text (stored as a linked list of blines)
typedef struct bline_s bline_t; // A line in a buffer
typedef struct bline_char_s bline_char_t; // Metadata about a character in a bline
typedef struct baction_s baction_t; // An insert or delete action (used for undo)
typedef struct mark_s mark_t; // A mark in a buffer
typedef struct srule_s srule_t; // A style rule
typedef struct srule_node_s srule_node_t; // A node in a list of style rules
typedef struct sblock_s sblock_t; // A style of a particular character
typedef void (*buffer_callback_t)(buffer_t* buffer, baction_t* action, void* udata);
typedef ssize_t bint_t;

// buffer_t
struct buffer_s {
    bline_t* first_line;
    bline_t* last_line;
    bint_t byte_count;
    bint_t char_count;
    bint_t line_count;
    srule_node_t* single_srules;
    srule_node_t* multi_srules;
    baction_t* actions;
    baction_t* action_tail;
    baction_t* action_undone;
    char* path;
    int is_unsaved;
    char *data;
    bint_t data_len;
    int is_data_dirty;
    int ref_count;
    int tab_width;
    buffer_callback_t callback;
    void* callback_udata;
    int is_in_callback;
    int is_style_disabled;
    char _mark_counter;
    int _is_in_undo;
};

// bline_t
struct bline_s {
    buffer_t* buffer;
    char* data;
    bint_t data_len;
    bint_t* data_vcols;
    bint_t data_cap;
    bint_t line_index;
    bint_t char_count;
    bint_t char_vwidth;
    bline_char_t* chars;
    bint_t chars_cap;
    sblock_t* char_styles;
    bint_t char_styles_cap;
    mark_t* marks;
    srule_t* bol_rule;
    srule_t* eol_rule;
    bline_t* next;
    bline_t* prev;
};

// bline_char_t
struct bline_char_s {
    uint32_t ch;
    int len;
    bint_t index;
    bint_t vcol;
};

// baction_t
struct baction_s {
    int type; // MLBUF_BACTION_TYPE_*
    buffer_t* buffer;
    bline_t* start_line;
    bint_t start_line_index;
    bint_t start_col;
    bline_t* maybe_end_line;
    bint_t maybe_end_line_index;
    bint_t maybe_end_col;
    bint_t byte_delta;
    bint_t char_delta;
    bint_t line_delta;
    char* data;
    bint_t data_len;
    baction_t* next;
    baction_t* prev;
};

// mark_t
struct mark_s {
    bline_t* bline;
    bint_t col;
    bint_t target_col;
    srule_t* range_srule;
    char letter;
    mark_t* next;
    mark_t* prev;
};

// sblock_t
struct sblock_s {
    uint16_t fg;
    uint16_t bg;
};

// srule_t
struct srule_s {
    int type; // MLBUF_SRULE_TYPE_*
    char* re;
    char* re_end;
    pcre* cre;
    pcre* cre_end;
    mark_t* range_a;
    mark_t* range_b;
    sblock_t style;
};
struct srule_node_s {
    srule_t* srule;
    srule_node_t* next;
    srule_node_t* prev;
};

// buffer functions
buffer_t* buffer_new();
buffer_t* buffer_new_open(char* path, int path_len);
mark_t* buffer_add_mark(buffer_t* self, bline_t* maybe_line, bint_t maybe_col);
int buffer_destroy_mark(buffer_t* self, mark_t* mark);
int buffer_open(buffer_t* self, char* path, int path_len);
int buffer_save(buffer_t* self);
int buffer_save_as(buffer_t* self, char* path, int path_len);
int buffer_get(buffer_t* self, char** ret_data, bint_t* ret_data_len);
int buffer_set(buffer_t* self, char* data, bint_t data_len);
int buffer_substr(buffer_t* self, bline_t* start_line, bint_t start_col, bline_t* end_line, bint_t end_col, char** ret_data, bint_t* ret_data_len, bint_t* ret_nchars);
int buffer_insert(buffer_t* self, bint_t offset, char* data, bint_t data_len, bint_t* optret_num_chars);
int buffer_delete(buffer_t* self, bint_t offset, bint_t num_chars);
int buffer_get_bline(buffer_t* self, bint_t line_index, bline_t** ret_bline);
int buffer_get_bline_col(buffer_t* self, bint_t offset, bline_t** ret_bline, bint_t* ret_col);
int buffer_get_offset(buffer_t* self, bline_t* bline, bint_t col, bint_t* ret_offset);
int buffer_undo(buffer_t* self);
int buffer_redo(buffer_t* self);
int buffer_add_srule(buffer_t* self, srule_t* srule);
int buffer_remove_srule(buffer_t* self, srule_t* srule);
int buffer_set_callback(buffer_t* self, buffer_callback_t cb, void* udata);
int buffer_set_tab_width(buffer_t* self, int tab_width);
int buffer_set_styles_enabled(buffer_t* self, int is_enabled);
int buffer_apply_styles(buffer_t* self, bline_t* start_line, bint_t line_delta);
int buffer_debug_dump(buffer_t* self, FILE* stream);
int buffer_destroy(buffer_t* self);
uintmax_t buffer_hash(buffer_t* self);

// bline functions
int bline_insert(bline_t* self, bint_t col, char* data, bint_t data_len, bint_t* ret_num_chars);
int bline_delete(bline_t* self, bint_t col, bint_t num_chars);
int bline_get_col(bline_t* self, bint_t index, bint_t* ret_col);

// mark functions
mark_t* mark_clone(mark_t* self);
int mark_insert_before(mark_t* self, char* data, bint_t data_len);
int mark_insert_after(mark_t* self, char* data, bint_t data_len);
int mark_delete_before(mark_t* self, bint_t num_chars);
int mark_delete_after(mark_t* self, bint_t num_chars);
int mark_move_to(mark_t* self, bint_t line_index, bint_t col);
int mark_move_by(mark_t* self, bint_t char_delta);
int mark_move_vert(mark_t* self, bint_t line_delta);
int mark_move_bol(mark_t* self);
int mark_move_eol(mark_t* self);
int mark_move_col(mark_t* self, bint_t col);
int mark_move_beginning(mark_t* self);
int mark_move_end(mark_t* self);
int mark_move_offset(mark_t* self, bint_t offset);
int mark_find_next_str(mark_t* self, char* str, bint_t str_len, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_prev_str(mark_t* self, char* str, bint_t str_len, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_next_cre(mark_t* self, pcre* cre, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_prev_cre(mark_t* self, pcre* cre, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_next_re(mark_t* self, char* re, bint_t re_len, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_prev_re(mark_t* self, char* re, bint_t re_len, bline_t** ret_line, bint_t* ret_col, bint_t* ret_num_chars);
int mark_find_bracket_pair(mark_t* self, bint_t max_chars, bline_t** ret_line, bint_t* ret_col, bint_t* ret_brkt);
int mark_find_bracket_top(mark_t* self, bint_t max_chars, bline_t** ret_line, bint_t* ret_col, bint_t* ret_brkt);
int mark_move_next_str(mark_t* self, char* str, bint_t str_len);
int mark_move_prev_str(mark_t* self, char* str, bint_t str_len);
int mark_move_next_cre(mark_t* self, pcre* cre);
int mark_move_prev_cre(mark_t* self, pcre* cre);
int mark_move_next_re(mark_t* self, char* re, bint_t re_len);
int mark_move_prev_re(mark_t* self, char* re, bint_t re_len);
int mark_move_bracket_pair(mark_t* self, bint_t max_chars);
int mark_move_bracket_top(mark_t* self, bint_t max_chars);
int mark_get_offset(mark_t* self, bint_t* ret_offset);
int mark_delete_between_mark(mark_t* self, mark_t* other);
int mark_get_between_mark(mark_t* self, mark_t* other, char** ret_str, bint_t* ret_str_len);
int mark_is_lt(mark_t* self, mark_t* other);
int mark_is_gt(mark_t* self, mark_t* other);
int mark_is_eq(mark_t* self, mark_t* other);
int mark_join(mark_t* self, mark_t* other);
int mark_swap_with_mark(mark_t* self, mark_t* other);
int mark_is_at_eol(mark_t* self);
int mark_is_at_bol(mark_t* self);
int mark_is_at_word_bound(mark_t* self, int side);
uint32_t mark_get_char_after(mark_t* self);
uint32_t mark_get_char_before(mark_t* self);
int mark_destroy(mark_t* self);

// srule functions
srule_t* srule_new_single(char* re, bint_t re_len, int caseless, uint16_t fg, uint16_t bg);
srule_t* srule_new_multi(char* re, bint_t re_len, char* re_end, bint_t re_end_len, uint16_t fg, uint16_t bg);
srule_t* srule_new_range(mark_t* range_a, mark_t* range_b, uint16_t fg, uint16_t bg);
int srule_destroy(srule_t* srule);

// utf8 functions
int utf8_char_length(char c);
int utf8_char_to_unicode(uint32_t *out, const char *c, const char *stop);
int utf8_unicode_to_char(char *out, uint32_t c);

// util functions
void* recalloc(void* ptr, size_t orig_num, size_t new_num, size_t el_size);
void _mark_mark_move_inner(mark_t* mark, bline_t* bline_target, bint_t col, int do_set_target, int do_style);

// Macros
#define MLBUF_DEBUG 1

#define MLBUF_OK 0
#define MLBUF_ERR 1

#define MLBUF_BACTION_TYPE_INSERT 0
#define MLBUF_BACTION_TYPE_DELETE 1

#define MLBUF_SRULE_TYPE_SINGLE 0
#define MLBUF_SRULE_TYPE_MULTI 1
#define MLBUF_SRULE_TYPE_RANGE 2

#define MLBUF_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MLBUF_MAX(a,b) (((a)>(b)) ? (a) : (b))

#define MLBUF_BLINE_DATA_STOP(bline) ((bline)->data + ((bline)->data_len))

#define MLBUF_DEBUG_PRINTF(fmt, ...) do { \
    if (MLBUF_DEBUG) { \
        fprintf(stderr, "%lu [%s] ", time(0), __PRETTY_FUNCTION__); \
        fprintf(stderr, (fmt), __VA_ARGS__); \
        fflush(stderr); \
    } \
} while (0)

#define MLBUF_MAKE_GT_EQ0(v) if ((v) < 0) v = 0

#define MLBUF_INIT_PCRE_EXTRA(n) \
    pcre_extra n = { .flags = PCRE_EXTRA_MATCH_LIMIT_RECURSION, .match_limit_recursion = 256 }

#endif
