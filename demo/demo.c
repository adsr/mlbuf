#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termbox.h>
#include "../mledit.h"
#include "assert.h"

buffer_t* buf;
mark_t* cursor;
char str[6];

void edit_init() {
    buf = buffer_new();
    cursor = buffer_add_mark(buf, NULL, 0);
}

void edit_type(struct tb_event* ev) {
    int len;
    len = tb_utf8_unicode_to_char(str, ev->ch);
    mark_insert_after(cursor, str, len);
}

void edit_delete(int backspace) {
    if (backspace) mark_delete_before(cursor, 1);
    else mark_delete_after(cursor, 1);
}

void edit_drop_mark() {
    buffer_add_mark(buf, cursor->bline, cursor->col);
}

void edit_render() {
    int w, h, i, j, is_mark;
    bline_t* line;
    mark_t* mark;
    w = tb_width();
    h = tb_height();
    for (line = buf->first_line, i = 0; line && i < h; line = line->next, i++) {
        snprintf(str, 5, "%lu", line->line_index);
        //fprintf(stderr, "line=%lu next=%lu prev=%lu\n", line->line_index, line->next ? line->next->line_index : 9999, line->prev ? line->prev->line_index : 9999 );
        for (j = 0; j < strlen(str) && j < 2; j++) tb_change_cell(j, i, *(str + j), 0, 0);
        for (; j < 2; j++) tb_change_cell(j, i, ' ', 0, 0);
        for (j = 0; j < w - 3 && j <= line->char_count; j++) {
            is_mark = 0;
            for (mark = line->marks; mark; mark = mark->next) {
                if (mark->col == j) {
                    is_mark = 1;
                    break;
                }
            }
            tb_change_cell(
                j+3, i,
                j < line->char_count ? line->data[line->char_indexes[j]] : ' ',
                0, is_mark ? TB_RED : 0
            );
        }
    }
    //fprintf(stderr, "first=%lu last=%lu\n", buf->first_line->line_index, buf->last_line->line_index);
    //fflush(stderr);
}

int main(int argc, char** argv) {
    struct tb_event ev;
    edit_init();
    tb_init();
    tb_select_input_mode(TB_MOD_ALT);
    edit_render();
    tb_present();
    while (tb_poll_event(&ev)) {
        if (ev.type == TB_EVENT_KEY) {
            if      (ev.key == TB_KEY_CTRL_C)      { break; }
            else if (ev.key == TB_KEY_CTRL_A)      { mark_move_bol(cursor); }
            else if (ev.key == TB_KEY_CTRL_E)      { mark_move_eol(cursor); }
            else if (ev.key == TB_KEY_CTRL_D)      { edit_drop_mark(); }
            else if (ev.key == TB_KEY_BACKSPACE)   { edit_delete(1); }
            else if (ev.key == TB_KEY_BACKSPACE2)  { edit_delete(1); }
            else if (ev.key == TB_KEY_ARROW_LEFT)  { mark_move_by(cursor, -1); }
            else if (ev.key == TB_KEY_ARROW_RIGHT) { mark_move_by(cursor, 1); }
            else if (ev.key == TB_KEY_ARROW_UP)    { mark_move_vert(cursor, -1); }
            else if (ev.key == TB_KEY_ARROW_DOWN)  { mark_move_vert(cursor, 1); }
            else if (ev.key == TB_KEY_DELETE)      { edit_delete(0); }
            else if (ev.key == TB_KEY_ENTER)       { mark_insert_after(cursor, "\n", 1); }
            else if (ev.key == TB_KEY_SPACE)       { mark_insert_after(cursor, " ", 1); }
            else if (ev.mod == 0 && ev.ch != 0)    { edit_type(&ev); }
            else { fprintf(stderr, "Unhandled event; key=%d\n", ev.key); }
        }
        tb_clear();
        edit_render();
        tb_present();
    }
    tb_shutdown();
    buffer_destroy(buf);
    exit(0);
}
