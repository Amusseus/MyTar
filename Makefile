SHELL = /bin/sh
CC = gcc
CFLAGS = -Wall -g
LD = gcc
LDFLAGS = -g
TARFILE = thistar2.tar

all: util.o mytar.o createTar.o readTar.o extractTar.o
	$(LD) $(LDFLAGS) -o mytar util.o mytar.o createTar.o readTar.o extractTar.o

mytar: util.o mytar.o createTar.o readTar.o extractTar.o
	$(LD) $(LDFLAGS) -o mytar util.o mytar.o createTar.o readTar.o extractTar.o

mytar.o: mytar.c
	$(CC) $(CFLAGS) -c -o mytar.o mytar.c

createTar.o: createTar.c
	$(CC) $(CFLAGS) -c -o createTar.o createTar.c

extractTar.o: extractTar.c util.o
	$(CC) $(CFLAGS) -c -o extractTar.o extractTar.c

readTar.o: readTar.c
	$(CC) $(CFLAGS) -c -o readTar.o readTar.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c -o util.o util.c

clean:
	rm *.o
	rm -f ./mytar
