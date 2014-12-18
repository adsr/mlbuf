CC="colorgcc"

all: libmledit.so

buffer.o: *.c
	$(CC) -Wall -g -fPIC -lpcre -c *.c

libmledit.a: buffer.o
	ar rcs libmledit.a *.o

libmledit.so: libmledit.a
	$(CC) -Wall -g -shared -lpcre -o libmledit.so *.o

test: libmledit.so
	make -C tests

demo: libmledit.so
	make -C demo

clean:
	rm -f *.o
	rm -f libmledit.a libmledit.so
	make -C tests clean
	make -C demo clean
