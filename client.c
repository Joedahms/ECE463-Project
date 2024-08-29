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

/*
 * Name: sendFile
 * Purpose: Send a file along with its name to a waiting server
 * Input: 
 * - The name of the file to send
 * - Socket Descriptor of the socket to send the file out on
 * Output: None
 */
void sendFile(const char* fileName, int socketDescriptor) {
	// Send the file name
	sendBytes(socketDescriptor, fileName, strlen(fileName), 0);

	int fileDescriptor;
	fileDescriptor = open(fileName, O_CREAT, O_RDWR);	// Create if does not exist + read and write mode

	struct stat fileInformation;
	stat(fileName, &fileInformation);
	unsigned long int fileSize = fileInformation.st_size;

	char* fileBuffer = malloc(100000);
	read(fileDescriptor, fileBuffer, fileSize);
	
	// Send the contents of the file
	sendBytes(socketDescriptor, fileBuffer, fileSize, 0);
}

/*
 * Name: sendBytes
 * Purpose: Send a desired number of bytes out on a Socket
 * Input:
 * - Socket Descriptor of the socket to send the bytes with
 * - Buffer containing the bytes to send
 * - Amount of bytes to send
 * - Flags? (lowkey don't remember why I put this here)
 * Output: None
 */
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

/*
 * Name: printFileInformation
 * Purpose: Utilize the stat data structure to print out various bits of
 * info about a particular file. Currently only using it to print out the
 * size of the file.
 * Input: 
 * - The name of the file
 * - The stat data structure corrosponding to the file
 * Output: None
 */
void printFileInformation(const char* fileName, struct stat fileInformation) {
	printf("Information about %s:\n", fileName);
	printf("Total size, in bytes: %ld\n", fileInformation.st_size);			
}
