CC="colorgcc"

all: libmlbuf.so

buffer.o: *.c
	$(CC) -Wall -g -fPIC -lpcre -c *.c

libmlbuf.a: buffer.o
	ar rcs libmlbuf.a *.o

libmlbuf.so: libmlbuf.a
	$(CC) -Wall -g -shared -lpcre -o libmlbuf.so *.o

test: libmlbuf.so
	make -C tests

demo: libmlbuf.so
	make -C demo

clean:
	rm -f *.o
	rm -f libmlbuf.a libmlbuf.so
	make -C tests clean
	make -C demo clean
