CC="colorgcc"

all: libmledit.so

buffer.o: *.c
	$(CC) -Wall -g -fPIC -c *.c

libmledit.a: buffer.o
	ar rcs libmledit.a *.o

libmledit.so: libmledit.a
	$(CC) -Wall -g -shared -o libmledit.so *.o

test: libmledit.so
	make -C tests

clean:
	rm -f *.o
	rm -f libmledit.a libmledit.so
	make -C tests clean
