CC=colorgcc

SRCS=$(wildcard *.c)
TESTS=$(SRCS:.c=)

all: run

run: $(TESTS)
	find . -type f -executable -print0 | xargs -0 -n1 -I{} sh -c 'tput bold; echo TEST {}; tput sgr0; LD_LIBRARY_PATH=.. {}; echo'

$(TESTS): $(SRCS)
	$(CC) -g -Wall -I.. -L.. $@.c -o $@ -lmlbuf -lpcre

clean:
	rm -f *.o
	find . -type f -executable -exec rm -f {} \;
