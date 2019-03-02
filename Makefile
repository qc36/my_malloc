CC=gcc
CFLAGS=-O0 -fPIC
DEPS=my_malloc.h

all: lib

lib: my_malloc.o
	$(CC) -g $(CFLAGS) -shared -o libmymalloc.so my_malloc.o

%.o: %.c my_malloc.h
	$(CC) -g $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *~ *.o *.so

clobber:
	rm -f *~ *.o
