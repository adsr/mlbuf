SHELL=/bin/bash
test_cflags:=$(CFLAGS) -D_GNU_SOURCE -Wall -g -I..
test_ldflags:=$(LDFLAGS) -L..
test_ldlibs:=$(LDLIBS) -lmlbuf -lpcre
test_bins:=$(patsubst %.c,%,$(wildcard *.c))

all: run

run: $(test_bins)
	total=0; \
	pass=0; \
	for t in $(test_bins); do \
		tput bold; echo TEST $$t; tput sgr0; \
		LD_LIBRARY_PATH=.. ./$$t; ec=$$?; echo; \
		[ $$ec -eq 0 ] && pass=$$((pass+1)); \
		total=$$((total+1)); \
	done; \
	printf "Passed %d out of %d tests\n" $$pass $$total

$(test_bins): %: %.c ../libmlbuf.so
	$(CC) $(test_cflags) $(test_ldflags) $< -o $@ $(test_ldlibs)

../libmlbuf.so:
	$(MAKE) -C ..

clean:
	rm -f $(test_bins)

.PHONY: all run clean
