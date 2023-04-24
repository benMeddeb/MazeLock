CC = gcc
CFLAGS = -Wall -pthread

all: mazelock

mazelock: mazelock.o
	$(CC) $(CFLAGS) -o $@ $^

mazelock.o: mazelock.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f mazelock mazelock.o
