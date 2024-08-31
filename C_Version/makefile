CFLAGS = -c -g

all: server client

server: server.o network_node.o
	gcc server.o network_node.o -o server

client: client.o network_node.o
	gcc client.o network_node.o -o client

client.o: client.c client.h
	gcc $(CFLAGS) client.c

server.o: server.c server.h 
	gcc $(CFLAGS) server.c

network_node.o: network_node.c network_node.h
	gcc $(CFLAGS) network_node.c

clean:
	rm client
	rm server
	rm *.o
