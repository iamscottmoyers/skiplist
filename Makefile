CC=gcc
CFLAGS=-O2 -g -Wall -pedantic -Wextra -std=gnu99

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS=
else
	LDFLAGS=-lrt
endif

default: skiplist

src/skiplist.o: src/skiplist.c src/skiplist.h src/skiplist_inline.h src/skiplist_types.h
	$(CC) -c $(CFLAGS) src/skiplist.c -o src/skiplist.o

skiplist: src/skiplist.o src/main.c
	$(CC) $(CFLAGS) src/main.c src/skiplist.o -o skiplist $(LDFLAGS)

test: skiplist
	./skiplist

html: Doxyfile src/skiplist.c src/skiplist.h src/skiplist_inline.h src/skiplist_types.h src/main.c
	doxygen

.PHONY: clean
clean:
	rm -f skiplist
	rm -f src/skiplist.o
	rm -rf skiplist.dSYM
	rm -rf html
