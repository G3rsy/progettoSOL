# Gabriele Sergi
#matrcicola 532362
all:
	gcc -pthread server.c -Wall -pedantic -fsanitize=address -o2 -fno-omit-frame-pointer -std=gnu99 -o server.o
	gcc client.c -Wall -pedantic -fsanitize=address -o2 -fno-omit-frame-pointer -std=gnu99 -o client.o
	gcc -pthread supervisor.c -Wall -pedantic -fsanitize=address -o2 -fno-omit-frame-pointer -std=gnu99 -o supervisor.o

clean:
	rm -f OOB* *.o

cleanall:
	rm -f OOB* *.o log*

test:
	make cleanall
	make all
	./run.sh
	make clean
