SHELL=/bin/bash
CC=$(shell if which colorgcc>/dev/null; then echo colorgcc; else echo gcc; fi)

all: run

run: demo
	LD_LIBRARY_PATH=.. ./demo 2>errlog

demo: demo.c
	$(CC) -g -Wall -I.. -L.. demo.c -o demo -lmlbuf -lpcre -ltermbox

clean:
	rm -f demo demo.o
