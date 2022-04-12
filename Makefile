# @auther: HASHIM
CC=gcc

CFLAGS=-c -Wall

all: Shell

Shell: Shell.o
	$(CC) Shell.o -o Shell

Shell.o: Shell.c
	$(CC) $(CFLAGS) Shell.c

clean:
	rm -rf *o Shell


