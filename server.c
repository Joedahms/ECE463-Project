#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

int receiveMessage(int, char*, int);
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
	fcntl(socketDescriptor, F_SETFD, O_NONBLOCK);

	bind(socketDescriptor, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen);
	
	listen(socketDescriptor, 10);		// Limit queued connections to 10

	struct sockaddr incomingAddress;
	int incomingSocketDescriptor;
	socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);

	// Continously listen for new files
	while (1) {
		incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);
		receiveFile(incomingSocketDescriptor);
	}
	
	freeaddrinfo(serverAddressInfo);
	return 0;
}

/*
 * Name: receiveMessage
 * Purpose: This function is for receiving a set number of bytes into
 * a buffer
 * Input: 
 * - Socket Descriptor of the accepted transmission
 * - Buffer to put the received data into
 * - The size of the message to receive in bytes
 * Output: 
 * - The number of bytes received into the buffer
 */
int receiveMessage(int incomingSocketDescriptor, char* incomingMessageBuffer, int messageSize) {
	// Recieve incoming message
	int bytesReceived = 0;
	bytesReceived = recv(incomingSocketDescriptor, incomingMessageBuffer, messageSize, 0);
	if (debugFlag) {
		printf("Bytes received in receiveMessage: %d\n", bytesReceived);
		// Print out incoming message
		int i;
		printf("Message received in receiveMessage: \n");
		for (i = 0; i < bytesReceived; i++) {
			printf("%c", incomingMessageBuffer[i]);
		}
		printf("\n");
	}
	return bytesReceived;
}

/*
 * Name: receiveFile
 * Purpose: This function is for reading a file into the present working directory.
 * The file name will be the same as the file that was sent.
 * Input: Socket Descriptor of the accepted transmission
 * Output: None
 */
void receiveFile(int incomingSocketDescriptor) {
	int bytesReceived;
	int i;

	char* fileName = malloc(20);
	bytesReceived = receiveMessage(incomingSocketDescriptor, fileName, 20);
	printf("Filename received: \n");
	for (i = 0; i < bytesReceived; i++) {
		printf("%c", fileName[i]);
	}
	printf("\n");

	char* fileContents = malloc(100000);
	bytesReceived = receiveMessage(incomingSocketDescriptor, fileContents, 100000);
	printf("File contents received: \n");
	for (i = 0; i < bytesReceived; i++) {
		printf("%c", fileContents[i]);
	}
	printf("\n");

	strcat(fileName, "_dupe");

	int receivedFile;
	receivedFile = open(fileName, (O_CREAT | O_RDWR), S_IRWXU);
	printf("%d\n", bytesReceived);
	write(receivedFile, fileContents, bytesReceived);
}
