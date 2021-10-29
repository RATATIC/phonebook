all : server

server : server.o log.o
	gcc -Wall server.o log.o -pthread -o server

server.o : server.c
	gcc -c server.c -pthread
	
log.o : log.c
	gcc -c log.c -pthread

clean :
	rm -rf *.o main
