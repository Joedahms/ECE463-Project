#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

void receiveFile(int);

int main(int argc, char* argv[]) {


	switch (argc) {					// Check how many command line arguments are passed
		case 1:
			printf("Running server in normal mode\n");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) {
				debugFlag = 1;
				printf("Running server in debug mode\n");
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

	receiveFile(socketDescriptor);
	
	freeaddrinfo(serverAddressInfo);

	return 0;
}

void receiveFile(int socketDescriptor) {
	struct sockaddr incomingAddress;
	int incomingSocketDescriptor;
	socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);
	incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);

	// Recieve incoming message
	char incomingMessageBuffer[100000];
	int sizeOfIncomingMessageBuffer = sizeof(incomingMessageBuffer);
	if (debugFlag) {
		printf("Size of incoming message buffer: %d\n", sizeOfIncomingMessageBuffer);
	}
	int bytesReceived = 0;
	bytesReceived = recv(incomingSocketDescriptor, incomingMessageBuffer, sizeOfIncomingMessageBuffer, 0);
	if (debugFlag) {
		printf("Bytes Received: %d\n", bytesReceived);
	}

	// Print out incoming message
	int i;
	printf("Incoming Message: \n");
	for (i = 0; i < bytesReceived; i++) {
		printf("%c", incomingMessageBuffer[i]);
	}
	printf("\n");
}
