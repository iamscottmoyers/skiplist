CC=gcc
CFLAGS=-O2 -g -Wall -pedantic -Wextra

default: skiplist

skiplist.o: skiplist.c skiplist.h skiplist_inline.h skiplist_types.h
	gcc -c $(CFLAGS) skiplist.c -o skiplist.o

skiplist: skiplist.o main.c
	gcc $(CFLAGS) main.c skiplist.o -o skiplist

test: skiplist
	./skiplist

html: Doxyfile skiplist.c skiplist.h skiplist_inline.h skiplist_types.h main.c
	doxygen

.PHONY: clean
clean:
	rm -f skiplist
	rm -f skiplist.o
	rm -rf skiplist.dSYM
	rm -rf html
