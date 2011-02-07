CC=gcc
CFLAGS=-g -pedantic -std=c99 -Wall -Wextra

opgave: dlist.o test.o bintree.o
	$(CC) $(CFLAGS) dlist.o test.o bintree.o -o a.out

bintree.o: bintree.c bintree.h
	$(CC) $(CFLAGS) -c bintree.c

dlist.o: dlist.c dlist.h bintree.h
	$(CC) $(CFLAGS) -c dlist.c

test.o: test.c bintree.h dlist.h
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -f *o
	rm -f a.out

all: opgave
