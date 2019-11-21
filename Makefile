# Gabriele Sergi
#matrcicola 532362
CC = gcc

CFLAGS = -Wall -pedantic -fsanitize=address -o2 -fno-omit-frame-pointer -std=gnu99

.PHONY:	all clean cleanall test

server.o: server.c include.h

client.o: client.c include.h

supervisor.o: supervisor.c include.h

all:
	$(CC) -pthread server.c  $(CFLAGS) -o server.o
	$(CC) client.c $(CFLAGS) -o client.o
	$(CC) -pthread supervisor.c $(CFLAGS) -o supervisor.o

clean:
	rm -f OOB* *.o

cleanall:
	rm -f OOB* *.o log*

test:
	make cleanall
	make all
	./run.sh
	make clean
