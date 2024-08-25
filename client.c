#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "client.h"

uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

void sendFile(const char*, int);
void sendBytes(int, const char*, unsigned long int, int);
void printFileInformation(const char*, struct stat);

int main(int argc, char* argv[]) {
	char* fileName = malloc(20);

	switch (argc) {					// Check how many command line arguments are passed
		case 1:
			printf("%s\n", "Running client in normal mode");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) {	// Check if debug flag
				debugFlag = 1;
				printf("%s\n", "Running client in debug mode");
			}
			else {					// Filename
				strcpy(fileName, argv[1]);
			}
			break;
		default:
	}
	
	int status;
	struct addrinfo hints;
	struct addrinfo* clientAddressInfo;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	getaddrinfo(NULL, "3940", &hints, &clientAddressInfo);

	int socketDescriptor;
	socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0);

	connect(socketDescriptor, clientAddressInfo->ai_addr, clientAddressInfo->ai_addrlen);

	sendFile(fileName, socketDescriptor);
	
	return 0;
}

void sendFile(const char* fileName, int socketDescriptor) {
	sendBytes(socketDescriptor, fileName, strlen(fileName), 0);

	int fileDescriptor;
	fileDescriptor = open(fileName, O_CREAT, O_RDWR);	// Create if does not exist + read and write mode

	struct stat fileInformation;
	stat(fileName, &fileInformation);
	unsigned long int fileSize = fileInformation.st_size;

	char* fileBuffer = malloc(100000);
	read(fileDescriptor, fileBuffer, fileSize);
	
	sendBytes(socketDescriptor, fileBuffer, fileSize, 0);
}

void sendBytes(int socketDescriptor, const char* fileBuffer, unsigned long int fileSize, int flags) {
	printf("Sending message: \n");
	int i;
	for (i = 0; i < fileSize; i++) {
		printf("%c", fileBuffer[i]);
	}
	printf("\n");

	int bytesSent = 0;
	bytesSent = send(socketDescriptor, fileBuffer, fileSize, 0);
	if (bytesSent != -1) {						// No error
		if (debugFlag) {
			printf("Bytes sent: %d\n", bytesSent);
		}	
	}
	else {
		printf("Error: send failed\n");
	}
}

void printFileInformation(const char* fileName, struct stat fileInformation) {
	printf("Information about %s:\n", fileName);
	printf("Total size, in bytes: %ld\n", fileInformation.st_size);			
}
