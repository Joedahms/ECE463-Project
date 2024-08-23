#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

int main(int argc, char* argv[]) {

	uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

	switch (argc) {					// Check how many command line arguments are passed
		case 1:
			printf("%s\n", "Running server in normal mode");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) {
				debugFlag = 1;
				printf("%s\n", "Running server in debug mode");
			}
			break;
		default:
	}

	int status;
	struct addrinfo hints;
	struct addrinfo* serverAddressInfo;

	hints.ai_family = AF_INET;		// IPV4
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_protocol = 0;			// Any protocol
	hints.ai_flags = AI_PASSIVE;		// If node is null, will bind to IP of host
	
	int getaddrinfoReturnValue;
	getaddrinfoReturnValue = getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);
	if (getaddrinfoReturnValue != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoReturnValue));	
		exit(EXIT_FAILURE);
	}

	int socketDescriptor;
	socketDescriptor = socket(serverAddressInfo->ai_family, serverAddressInfo->ai_socktype, 0);

	bind(socketDescriptor, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen);
	
	listen(socketDescriptor, 10);		// Limit queued connections to 10

	struct sockaddr incomingAddress;
	int incomingSocketDescriptor;
	socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);
	incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);

	char incomingBuffer[1000];
	recv(incomingSocketDescriptor, incomingBuffer, strlen(incomingBuffer), 0);

	int i;
	for (i = 0; i < 1000; i++) {
		printf("%c", incomingBuffer[i]);
	}
	
	freeaddrinfo(serverAddressInfo);

	return 0;
}
