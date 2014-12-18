#ifndef __MLEDIT_H
#define __MLEDIT_H

#include <stdio.h>
#include <stdint.h>
#include <pcre.h>
#include "utlist.h"

// Typedefs
typedef struct buffer_s buffer_t; // A buffer of text (stored as a linked list of blines)
typedef struct bline_s bline_t; // A line in a buffer
typedef struct baction_s baction_t; // An insert or delete action (used for undo)
typedef struct bevent_s bevent_t; // A buffer event passed to all blisteners
typedef struct blistener_s blistener_t; // A listener of bevents
typedef struct mark_s mark_t; // A mark in a buffer
typedef struct srule_s srule_t; // A style rule
typedef struct srule_node_s srule_node_t; // A node in a list of style rules
typedef struct sblock_s sblock_t; // A style of a particular character

// bevent_t
struct bevent_s {
    int type; // MLEDIT_BEVENT_TYPE_*
    baction_t* baction;
    bline_t* bline;
};

// buffer_t
struct buffer_s {
    bline_t* first_line;
    bline_t* last_line;
    size_t byte_count;
    size_t char_count;
    size_t line_count;
    srule_node_t* single_srules;
    srule_node_t* multi_srules;
    blistener_t* listeners;
    baction_t* actions;
    baction_t* action_tail;
    baction_t* action_undone;
    char *data;
    size_t data_len;
    int is_data_dirty;
    int _is_in_event_cb;
    char _mark_counter;
    blistener_t* _listener_tmp;
    bevent_t _event_tmp;
};

// bline_t
struct bline_s {
    buffer_t* buffer;
    char* data;
    size_t data_len;
    size_t data_cap;
    size_t line_index;
    size_t char_count;
    size_t* char_indexes;
    size_t char_indexes_cap;
    sblock_t* char_styles;
    size_t char_styles_cap;
    mark_t* marks;
    srule_t* bol_rule;
    srule_t* eol_rule;
    void* udata;
    bline_t* next;
    bline_t* prev;
};

// baction_t
struct baction_s {
    int type; // MLEDIT_BACTION_TYPE_*
    buffer_t* buffer;
    bline_t* start_line;
    size_t start_line_index;
    size_t start_col;
    bline_t* maybe_end_line;
    size_t maybe_end_line_index;
    size_t maybe_end_col;
    ssize_t byte_delta;
    ssize_t char_delta;
    ssize_t line_delta;
    char* data;
    size_t data_len;
    baction_t* next;
    baction_t* prev;
};

// blistener_t
typedef void (*blistener_callback_t) (
    void* listener,
    buffer_t* buffer,
    bevent_t bevent
);
struct blistener_s {
    void* listener;
    blistener_callback_t callback;
    blistener_t* next;
    blistener_t* prev;
};

// mark_t
struct mark_s {
    bline_t* bline;
    size_t col;
    size_t target_col;
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
    int type; // MLEDIT_SRULE_TYPE_*
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
mark_t* buffer_add_mark(buffer_t* self, bline_t* maybe_line, size_t maybe_col);
int buffer_get(buffer_t* self, char** ret_data, size_t* ret_data_len);
int buffer_set(buffer_t* self, char* data, size_t data_len);
int buffer_substr(buffer_t* self, bline_t* start_line, size_t start_col, bline_t* end_line, size_t end_col, char** ret_data, size_t* ret_data_len, size_t* ret_nchars);
int buffer_insert(buffer_t* self, size_t offset, char* data, size_t data_len, size_t* ret_num_chars);
int buffer_delete(buffer_t* self, size_t offset, size_t num_chars);
int buffer_get_bline_col(buffer_t* self, size_t offset, bline_t** ret_bline, size_t* ret_col);
int buffer_get_offset(buffer_t* self, bline_t* bline, size_t col, size_t* ret_offset);
int buffer_undo(buffer_t* self);
int buffer_redo(buffer_t* self);
int buffer_repeat_at(buffer_t* self, size_t offset);
int buffer_add_srule(buffer_t* self, srule_t* srule);
int buffer_remove_srule(buffer_t* self, srule_t* srule);
int buffer_add_listener(buffer_t* self, blistener_t blistener);
int buffer_debug_dump(buffer_t* self, FILE* stream);
int buffer_destroy(buffer_t* self);

// bline functions
int bline_insert(bline_t* self, size_t col, char* data, size_t data_len, size_t* ret_num_chars);
int bline_delete(bline_t* self, size_t col, size_t num_chars);
int bline_get_col(bline_t* self, size_t index, size_t* ret_col);

// blistener functions
blistener_t* blistener_new(void* listener, blistener_callback_t cb);
int blistener_destroy(blistener_t* self);

// mark functions
mark_t* mark_clone(mark_t* self);
int mark_insert_before(mark_t* self, char* data, size_t data_len);
int mark_insert_after(mark_t* self, char* data, size_t data_len);
int mark_delete_before(mark_t* self, size_t num_chars);
int mark_delete_after(mark_t* self, size_t num_chars);
int mark_move_by(mark_t* self, ssize_t char_delta);
int mark_move_vert(mark_t* self, ssize_t line_delta);
int mark_move_bol(mark_t* self);
int mark_move_eol(mark_t* self);
int mark_move_col(mark_t* self, size_t col);
int mark_move_beginning(mark_t* self);
int mark_move_end(mark_t* self);
int mark_move_offset(mark_t* self, size_t offset);
int mark_find_next_str(mark_t* self, char* str, size_t str_len, bline_t** ret_line, size_t* ret_col);
int mark_find_prev_str(mark_t* self, char* str, size_t str_len, bline_t** ret_line, size_t* ret_col);
int mark_find_next_cre(mark_t* self, pcre* cre, bline_t** ret_line, size_t* ret_col);
int mark_find_prev_cre(mark_t* self, pcre* cre, bline_t** ret_line, size_t* ret_col);
int mark_find_next_re(mark_t* self, char* re, size_t re_len, bline_t** ret_line, size_t* ret_col);
int mark_find_prev_re(mark_t* self, char* re, size_t re_len, bline_t** ret_line, size_t* ret_col);
int mark_find_bracket_pair(mark_t* self, size_t max_chars, bline_t** ret_line, size_t* ret_col);
int mark_move_next_str(mark_t* self, char* str, size_t str_len);
int mark_move_prev_str(mark_t* self, char* str, size_t str_len);
int mark_move_next_cre(mark_t* self, pcre* cre);
int mark_move_prev_cre(mark_t* self, pcre* cre);
int mark_move_next_re(mark_t* self, char* re, size_t re_len);
int mark_move_prev_re(mark_t* self, char* re, size_t re_len);
int mark_move_bracket_pair(mark_t* self, size_t max_chars);
int mark_get_offset(mark_t* self, size_t* ret_offset);
int mark_delete_between_mark(mark_t* self, mark_t* other);
int mark_get_between_mark(mark_t* self, mark_t* other, char** ret_str, size_t* ret_str_len);
int mark_is_gt(mark_t* self, mark_t* other);
int mark_swap_with_mark(mark_t* self, mark_t* other);

// srule functions
srule_t* srule_new_single(char* re, size_t re_len, uint16_t fg, uint16_t bg);
srule_t* srule_new_multi(char* re, size_t re_len, char* re_end, size_t re_end_len, uint16_t fg, uint16_t bg);
srule_t* srule_new_range(mark_t* range_a, mark_t* range_b, uint16_t fg, uint16_t bg);
int srule_destroy(srule_t* srule);

// utf8 functions
int utf8_char_length(char c);
int utf8_char_to_unicode(uint32_t *out, const char *c);
int utf8_unicode_to_char(char *out, uint32_t c);

// Macros
#define MLEDIT_DEBUG 1

#define MLEDIT_OK 0
#define MLEDIT_ERR 1

#define MLEDIT_BACTION_TYPE_INSERT 0
#define MLEDIT_BACTION_TYPE_DELETE 1

#define MLEDIT_BEVENT_TYPE_BACTION 0
#define MLEDIT_BEVENT_TYPE_BLINE_CTOR 1
#define MLEDIT_BEVENT_TYPE_BLINE_DTOR 2
#define MLEDIT_BEVENT_TYPE_BLINE_UPDATE 3
#define MLEDIT_BEVENT_RAISE(p_buffer, p_type, p_bevent) do { \
    if ((p_buffer)->_is_in_event_cb != 0) break; \
    memset(&((p_buffer)->_event_tmp), 0, sizeof(bevent_t)); \
    (p_buffer)->_event_tmp = (struct bevent_t){(p_bevent)}; \
    (p_buffer)->_event_tmp.type = (p_type); \
    (p_buffer)->_is_in_event_cb = 1; \
    DL_FOREACH((p_buffer)->blisteners, (p_buffer)->_listener_tmp) { \
        ((p_buffer)->_listener_tmp->callback)( \
            (p_buffer)->_listener_tmp->listener, \
            (p_buffer), \
            (p_buffer)->_event_tmp \
        ); \
    } \
    (p_buffer)->_is_in_event_cb = 0; \
} while(0)

#define MLEDIT_SRULE_TYPE_SINGLE 0
#define MLEDIT_SRULE_TYPE_MULTI 1
#define MLEDIT_SRULE_TYPE_RANGE 2

#define MLEDIT_MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MLEDIT_MAX(a,b) (((a)>(b)) ? (a) : (b))

#define MLEDIT_BLINE_DATA_STOP(bline) ((bline)->data + ((bline)->data_len))

#define MLEDIT_MARK_MOVE(pmark, ptarget, pcol, psettarg) do { \
    DL_DELETE((pmark)->bline->marks, (pmark)); \
    (pmark)->bline = (ptarget); \
    MLEDIT_MARK_SET_COL((pmark), (pcol), (psettarg)); \
    DL_APPEND((ptarget)->marks, (pmark)); \
} while(0);

#define MLEDIT_MARK_SET_COL(pmark, pcol, psettarg) do { \
    (pmark)->col = MLEDIT_MIN((pmark)->bline->char_count, MLEDIT_MAX(0, (pcol))); \
    if (psettarg) (pmark)->target_col = (pmark)->col; \
} while(0);

#define MLEDIT_DEBUG_PRINTF(fmt, ...) do { \
    if (MLEDIT_DEBUG) { \
        fprintf(stderr, "%lu [%s] ", time(0), __PRETTY_FUNCTION__); \
        fprintf(stderr, (fmt), __VA_ARGS__); \
        fflush(stderr); \
    } \
} while (0)

#endif
