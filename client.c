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

void printFileInformation(const char*, struct stat);

int main(int argc, char* argv[]) {
	uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info
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

	int fileDescriptor;
	fileDescriptor = open(fileName, O_CREAT, O_RDWR);

	struct stat fileInformation;
	stat(fileName, &fileInformation);

	int i;
	unsigned long int fileSize = fileInformation.st_size;
	char* fileBuffer = malloc(100000);
	read(fileDescriptor, fileBuffer, fileSize);
	for (i = 0; i < fileSize; i++) {
		printf("%c", fileBuffer[i]);
	}

	//char outgoingData[] = "TestingTestingTesting";
	int bytesSent = 0;
	//bytesSent = send(socketDescriptor, outgoingData, strlen(outgoingData), 0);
	bytesSent = send(socketDescriptor, fileBuffer, fileSize, 0);
	if (bytesSent != -1) {
		if (debugFlag) {
			printf("Bytes sent: %d\n", bytesSent);
		}	
	}
	else {
		printf("Error: send failed");
	}

	return 0;
}

void printFileInformation(const char* fileName, struct stat fileInformation) {
	printf("Information about %s:\n", fileName);
	printf("Total size, in bytes: %ld\n", fileInformation.st_size);			
}
