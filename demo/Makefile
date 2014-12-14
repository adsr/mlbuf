CC=colorgcc

all: run

run: demo
	LD_LIBRARY_PATH=.. ./demo 2>errlog

demo: demo.c
	$(CC) -g -Wall -I.. -L.. demo.c -o demo -lmledit -lpcre -ltermbox

clean:
	rm -f demo demo.o
