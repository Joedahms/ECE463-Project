CFLAGS = -c -g
S = server_code/
CL = client_code/
CO = common/

all: server client

server: server.o network_node.o
	gcc server.o network_node.o -o server
	mv server server_code

client: client.o network_node.o
	gcc client.o network_node.o -o client
	mv client client_code

client.o: $(CL)client.c $(CL)client.h
	gcc $(CFLAGS) $(CL)client.c

server.o: $(S)server.c $(S)server.h 
	gcc $(CFLAGS) $(S)server.c

network_node.o: $(CO)network_node.c $(CO)network_node.h
	gcc $(CFLAGS) $(CO)network_node.c

clean:
	rm client_code/client
	rm server_code/server
	rm *.o
