#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "network_node.h"
#include "server.h"

// Global flags
uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
struct addrinfo* serverAddressInfo;
int socketDescriptor;
int incomingSocketDescriptor;

// Forward declarations
//int receiveMessage(int, char*, int);
//void receiveFile(int);
void shutdownServer(int);

// Main fucntion
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownServer);

  // Check how many command line areguements passed
	switch (argc) {
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

	hints.ai_family = AF_INET;        // IPV4
	hints.ai_socktype = SOCK_STREAM;  // TCP
	hints.ai_protocol = 0;            // Any protocol
	hints.ai_flags = AI_PASSIVE;      // If node is null, will bind to IP of host
	
	int getaddrinfoReturnValue;
	getaddrinfoReturnValue = getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);
	if (getaddrinfoReturnValue != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoReturnValue));	
		exit(EXIT_FAILURE);
	}
  
  printf("Setting up socket...\n");
  socketDescriptor = socket(serverAddressInfo->ai_family, serverAddressInfo->ai_socktype, 0);
  fcntl(socketDescriptor, F_SETFD, O_NONBLOCK);
  printf("Socket set up\n");

  printf("Binding socket...\n");
  bind(socketDescriptor, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen);
  printf("Socket bound\n");
	
  listen(socketDescriptor, 10);		// Limit queued connections to 10
  printf("Listening...\n");

  struct sockaddr incomingAddress;
  int incomingSocketDescriptor;
  socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);

	// Continously listen for new files
  while (1) {
    incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);
    printf("Connection accepted\n");
    receiveFile(incomingSocketDescriptor, debugFlag);
    close(incomingSocketDescriptor);
    printf("Connection terminated\n");
  }
	
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
/*
int receiveBytes(int incomingSocketDescriptor, char* buffer, int bufferSize) {
  printf("Receiving bytes...\n");
	int numberOfBytesReceived = 0;
	numberOfBytesReceived = recv(incomingSocketDescriptor, buffer, bufferSize, 0);
	if (debugFlag) {
		// Print out incoming message
		int i;
		printf("Bytes received: \n");
		for (i = 0; i < numberOfBytesReceived; i++) {
			printf("%c", buffer[i]);
		}
		printf("\n");
	}
  printf("%d bytes received\n", numberOfBytesReceived);
	return numberOfBytesReceived;
}
*/

/*
 * Name: receiveFile
 * Purpose: This function is for reading a file into the present working directory.
 * The file name will be the same as the file that was sent.
 * Input: Socket Descriptor of the accepted transmission
 * Output: None
 */
/*
void receiveFile(int incomingSocketDescriptor) {
  printf("Receiving File...\n");

	int bytesReceived;
	int i;

  // Receive file name
  printf("Receiving file name...\n");
	char* receivedFileName = malloc(20);
	bytesReceived = receiveBytes(incomingSocketDescriptor, receivedFileName, 20);
	printf("Filename received: %s\n", receivedFileName);

  // Receive file contents
  printf("Receiving file contents...\n");
	char* fileContents = malloc(1000);
	bytesReceived = receiveBytes(incomingSocketDescriptor, fileContents, 1000);
	printf("File contents received: \n");

  // Change the filename so that the received file is put in the test directory
  char fileName[30] = "test/";
  strcat(fileName, receivedFileName); 

  // Open and write to the new file
  int receivedFile;
  printf("Opening received file...\n");
	receivedFile = open(fileName, (O_CREAT | O_RDWR), S_IRWXU);
  printf("Received file opened\n");
  printf("Writing received file...\n");
	write(receivedFile, fileContents, bytesReceived);
  printf("Received file written\n");
  
  printf("File received\n");
}
*/

/*
* Name: shutdownServer
* Purpose: Gracefully shutdown the server when the user enters
* ctrl-c. Closes the sockets and frees addrinfo data structure
* Input: The signal raised
* Output: None
*/
void shutdownServer(int signal) {
  close(incomingSocketDescriptor);
  close(socketDescriptor);
	freeaddrinfo(serverAddressInfo);
  printf("\n");
  exit(0);
}
