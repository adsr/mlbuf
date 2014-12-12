CC="colorgcc"

all: run

SRCS=$(wildcard *.c)
TESTS=$(patsubst %.c,%,$(SRCS))

run: $(TESTS)
	find . -type f -executable -print0 | xargs -0 -n1 -I{} sh -c 'tput bold; echo TEST {}; tput sgr0; {}; echo'

$(TESTS): $(SRCS)
	gcc -Wall -g -static -L.. -I.. $< -o $@ -lmledit

clean:
	rm -f *.o
	find . -type f -executable -exec rm -f {} \;
