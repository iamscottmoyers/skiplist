CC=gcc
CFLAGS=-O2 -Wall -pedantic -Wextra


all: skiplist html

skiplist.o: skiplist.c skiplist.h skiplist_inline.h skiplist_types.h
	gcc -c $(CFLAGS) skiplist.c -o skiplist.o

skiplist: skiplist.o main.c
	gcc $(CFLAGS) main.c skiplist.o -o skiplist

test: skiplist
	./skiplist

html: Doxyfile skiplist.c skiplist.h main.c
	doxygen

.PHONY: clean
clean:
	rm -f skiplist
	rm -f skiplist.o
	rm -rf html
