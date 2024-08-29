CFLAGS = -c -g

all: server client

server: server.o
	gcc server.o -o server

client: client.o
	gcc client.o -o client

client.o: client.c client.h
	gcc $(CFLAGS) client.c

server.o: server.c server.h
	gcc $(CFLAGS) server.c

clean:
	rm client
	rm server
	rm *.o
