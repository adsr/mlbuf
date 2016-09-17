#include <stdlib.h>
#include <string.h>
#include "mlbuf.h"

// Zero-fill realloc
void* recalloc(void* ptr, size_t orig_num, size_t new_num, size_t el_size) {
    void* newptr;
    newptr = realloc(ptr, new_num * el_size);
    if (!newptr) return NULL;
    if (new_num > orig_num) {
        memset(newptr + (orig_num * el_size), 0, (new_num - orig_num) * el_size);
    }
    return newptr;
}

// Append from data up until data_stop to str
void str_append_stop(str_t* str, char* data, char* data_stop) {
    size_t data_len;
    data_len = data_stop >= data ? data_stop - data : 0;
    str_append_len(str, data, data_len);
}

// Append data to str
void str_append(str_t* str, char* data) {
    str_append_len(str, data, strlen(data));
}

// Append data_len bytes of data to str
void str_append_len(str_t* str, char* data, size_t data_len) {
    str_put_len(str, data, data_len, 0);
}

// Prepend from data up until data_stop to str
void str_prepend_stop(str_t* str, char* data, char* data_stop) {
    size_t data_len;
    data_len = data_stop >= data ? data_stop - data : 0;
    str_prepend_len(str, data, data_len);
}

// Prepend data to str
void str_prepend(str_t* str, char* data) {
    str_prepend_len(str, data, strlen(data));
}

// Prepend data_len bytes of data to str
void str_prepend_len(str_t* str, char* data, size_t data_len) {
    str_put_len(str, data, data_len, 1);
}

// Set str to data
void str_set(str_t* str, char* data) {
    str_set_len(str, data, strlen(data));
}

// Set str to data for data_len bytes
void str_set_len(str_t* str, char* data, size_t data_len) {
    str_ensure_cap(str, data_len+1);
    memcpy(str->data, data, data_len);
    str->len = data_len;
    *(str->data + str->len) = '\0';
}

// Append/prepend data_len bytes of data to str
void str_put_len(str_t* str, char* data, size_t data_len, int is_prepend) {
    size_t req_cap;
    req_cap = str->len + data_len + 1;
    if (req_cap > str->cap) {
        req_cap = MLBUF_MAX(req_cap, str->cap + (str->inc > 0 ? str->inc : 128));
    }
    str_ensure_cap(str, req_cap);;
    if (is_prepend) {
        memmove(str->data, str->data + data_len, data_len);
        memcpy(str->data, data, data_len);
    } else {
        memcpy(str->data + str->len, data, data_len);
    }
    str->len += data_len;
    *(str->data + str->len) = '\0';
}

// Ensure space in str
void str_ensure_cap(str_t* str, size_t cap) {
    if (cap > str->cap) {
        str->cap = cap;
        str->data = realloc(str->data, str->cap);
    }
}

// Clear str
void str_clear(str_t* str) {
    str->len = 0;
}

// Free str
void str_free(str_t* str) {
    if (str->data) free(str->data);
    memset(str, 0, sizeof(str_t));
}

// Replace `repl` in `subj` and append result to `str`. PCRE style backrefs are
// supported.
//
//   str           where to append data
//   subj          subject string
//   repl          replacement string with $1 or \1 style backrefs
//   pcre_rc       return code from pcre_exec
//   pcre_ovector  ovector used with pcre_exec
//   pcre_ovecsize size of pcre_ovector
//
void str_append_replace_with_backrefs(str_t* str, char* subj, char* repl, int pcre_rc, int* pcre_ovector, int pcre_ovecsize) {
    char* repl_stop;
    char* repl_cur;
    char* repl_z;
    char* repl_backref;
    int ibackref;
    char* term;
    char* term_stop;

    repl_stop = repl + strlen(repl);

    // Start replace loop
    repl_cur = repl;
    while (repl_cur < repl_stop) {
        // Find backref marker (dollar sign or backslash) in replacement str
        repl_backref = strpbrk(repl_cur, "$\\");
        repl_z = repl_backref ? repl_backref : repl_stop;

        // Append part before backref
        str_append_stop(str, repl_cur, repl_z);

        // Break if no backref
        if (!repl_backref) break;

        // Append backref
        term = NULL;
        if (repl_backref+1 >= repl_stop) {
            // No data after backref marker; append the marker itself
            term = repl_backref;
            term_stop = repl_stop;
        } else if (*(repl_backref+1) >= '0' && *(repl_backref+1) <= '9') {
            // N was a number; append Nth captured substring from match
            ibackref = *(repl_backref+1) - '0';
            if (ibackref < pcre_rc && ibackref < pcre_ovecsize/3) {
                // Backref exists
                term = subj + pcre_ovector[ibackref*2];
                term_stop = subj + pcre_ovector[ibackref*2 + 1];
            } else {
                // Backref does not exist; append marker + whatever character it was
                term = repl_backref;
                term_stop = term + utf8_char_length(*(term+1));
            }
        } else {
            // N was not a number; append marker + whatever character it was
            term = repl_backref;
            term_stop = term + utf8_char_length(*(term+1));
        }
        str_append_stop(str, term, term_stop);

        // Advance repl_cur by 2 bytes (marker + backref num)
        repl_cur = repl_backref+2;
    }
}
