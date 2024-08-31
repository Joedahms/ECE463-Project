#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>


#include "network_node.h"

/*
 * Name: sendFile
 * Purpose: Send a file along with its name to a waiting server
 * Input: 
 * - The name of the file to send
 * - Socket Descriptor of the socket to send the file out on
 * Output: None
 */
void sendFile(const char* fileName, int socketDescriptor, uint8_t debugFlag) {
	// Send the file name
  int fileNameLength = strlen(fileName);
  printf("Sending file: %s\n", fileName);
  if (debugFlag) {
    printf("Length of file name: %d\n", fileNameLength);
  }
	sendBytes(socketDescriptor, fileName, strlen(fileName), debugFlag);

  // Open the file
	int fileDescriptor;
  printf("Opening file...\n");
	fileDescriptor = open(fileName, O_CREAT, O_RDWR);	// Create if does not exist + read and write mode
  printf("File Open\n");

  // Get the size of the file in bytes
	struct stat fileInformation;
	if (stat(fileName, &fileInformation) == -1) {
    printf("Stat Error\n");
    exit(1);
  };
	unsigned long int fileSize = fileInformation.st_size;

  // Read the contents of the file into the file buffer
	char* fileBuffer = malloc(100000);
  printf("Reading File...\n");
	read(fileDescriptor, fileBuffer, fileSize);
  printf("File Read\n");

	// Send the contents of the file
  printf("Sending file contents...\n");
	sendBytes(socketDescriptor, fileBuffer, fileSize, debugFlag);
  printf("File contents sent\n");
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
 * Notes: need to change variable names to be more ambiguous
 */
void sendBytes(int socketDescriptor, const char* fileBuffer, unsigned long int fileSize, uint8_t debugFlag) {
  if (debugFlag) {
    printf("File size: %ld\n", fileSize);
  }
  // Print the bytes to send
  printf("Bytes to be sent:\n\n");
	int i;
	for (i = 0; i < fileSize; i++) {
		printf("%c", fileBuffer[i]);
	}
  printf("\n \n");

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
int receiveBytes(int incomingSocketDescriptor, char* buffer, int bufferSize, uint8_t debugFlag) {
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

/*
 * Name: receiveFile
 * Purpose: This function is for reading a file into the present working directory.
 * The file name will be the same as the file that was sent.
 * Input: Socket Descriptor of the accepted transmission
 * Output: None
 */
void receiveFile(int incomingSocketDescriptor, uint8_t debugFlag) {
  printf("Receiving File...\n");

	int bytesReceived;
	int i;

  // Receive file name
  printf("Receiving file name...\n");
	char* receivedFileName = malloc(20);
	bytesReceived = receiveBytes(incomingSocketDescriptor, receivedFileName, 20, debugFlag);
	printf("Filename received: %s\n", receivedFileName);

  // Receive file contents
  printf("Receiving file contents...\n");
	char* fileContents = malloc(1000);
	bytesReceived = receiveBytes(incomingSocketDescriptor, fileContents, 1000, debugFlag);
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


