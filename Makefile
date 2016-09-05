SHELL=/bin/bash
mlbuf_cflags:=$(CFLAGS) -D_GNU_SOURCE -Wall -g -fPIC
mlbuf_ldlibs:=$(LDLIBS) -lpcre
mlbuf_objects:=$(patsubst %.c,%.o,$(wildcard *.c))

all: libmlbuf.so

libmlbuf.so: libmlbuf.a
	$(CC) -shared $(mlbuf_ldlibs) -o libmlbuf.so $(mlbuf_objects)

libmlbuf.a: $(mlbuf_objects)
	$(AR) rcs libmlbuf.a $(mlbuf_objects)

$(mlbuf_objects): %.o: %.c
	$(CC) -c $(mlbuf_cflags) $< -o $@

test: libmlbuf.so
	$(MAKE) -C tests

clean:
	rm -f *o libmlbuf.a libmlbuf.so
	$(MAKE) -C tests clean

.PHONY: all test clean
