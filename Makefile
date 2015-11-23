CC=gcc
CFLAGS=-O2 -Wall -pedantic -Wextra


all: skiplist-test doc

skiplist.o: skiplist.c skiplist.h
	gcc -c $(CFLAGS) skiplist.c -o skiplist.o

skiplist-test: skiplist.o skiplist.h main.c
	gcc $(CFLAGS) main.c skiplist.o -o skiplist-test

doc: Doxyfile skiplist.c skiplist.h main.c
	doxygen

.PHONY: clean
clean:
	rm -f skiplist-test
	rm -f skiplist.o
	rm -rf html latex
