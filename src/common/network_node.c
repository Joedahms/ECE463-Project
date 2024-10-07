#define MAX_PACKET_LENGTH 5000  // Upper limit on packet size (bytes)

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "network_node.h"

// Check how many command line arguments passed
void checkCommandLineArguments(int argc, char** argv, uint8_t* debugFlag) {
  char* programName = argv[0];
	switch (argc) { // Check how many command line arguments are passed
		case 1:
			printf("Running %s in normal mode\n", programName);
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) { // Check if debug flag
				*debugFlag = 1;
				printf("Running %s in debug mode\n", programName);
			}
			else {
				printf("Invalid usage of %s", programName);
			}
			break;
    default:
			printf("Invalid usage of %s", programName);
	}
}

/*
 * Name: networkNodeConnect
 * Purpose: Connect to another IPV4 socket via TCP
 * Input: 
 * - Name of the node to connect to
 * - Socket on calling node process
 * - addrinfo structure containing information about node to connect to
 * Output: 
 * - Connected socket descriptor
 */
int networkNodeConnect(const char* nodeName, int socketDescriptor, struct sockaddr* destinationAddress, socklen_t destinationAddressLength) {
  printf("Connecting to %s...\n", nodeName);
  int connectionStatus;
  connectionStatus = connect(socketDescriptor, destinationAddress, destinationAddressLength);
  // Check if connection was successful
  if (connectionStatus != 0) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Connection to %s failed with error %s\n", nodeName, errorMessage);
    exit(1);
  }
  printf("Connected to %s...\n", nodeName);
  return socketDescriptor;
}

/*
 * Name: sendBytes
 * Purpose: Send a desired number of bytes out on a Socket
 * Input:
 * - Socket Descriptor of the socket to send the bytes with
 * - Buffer containing the bytes to send
 * - Amount of bytes to send
 * - Debug flag
 * Output: Number of bytes sent
 */
int sendBytes(int socketDescriptor, const char* buffer, unsigned long int bufferSize, uint8_t debugFlag) {
  if (debugFlag) {
    printf("Bytes to be sent:\n\n");
    int i;
    for (i = 0; i < bufferSize; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n\n");
  }

  int bytesSent = 0;
  bytesSent = send(socketDescriptor, buffer, bufferSize, 0);
  if (bytesSent == -1) {  
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Byte send failed with error %s\n", errorMessage);
    exit(1);
  }
  else {
    return bytesSent;
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
  return numberOfBytesReceived;
}

/*
 * Name: checkStringForCommand
 * Purpose: Check if a string has a command in it
 * Input: String that might have a command
 * Ouptut: 
 * - 0: String is not a command
 * - 1: String is a command
 */
int checkStringForCommand(const char* userInput) {
  if (userInput[0] == '%') {  // Check first character for '%'
    return 1; // User entered command
  }
  else {
    return 0; // User entered plain text
  }
}

/*
  * Name:
  * Purpose:
  * Input: 
  * Output:
*/

void printReceivedMessage(struct sockaddr_in sender, int bytesReceived, char* message, uint8_t debugFlag) {
  if (debugFlag) {
    unsigned long senderAddress = ntohl(sender.sin_addr.s_addr);
    unsigned short senderPort = ntohs(sender.sin_port);
    printf("Received %d byte message from %ld:%d:\n", bytesReceived, senderAddress, senderPort);
    printf("%s\n", message);
  }
  else {
    printf("Received %d byte message\n", bytesReceived);
  }
}

/*
  * Name: readFile
  * Purpose: Open a file and read from it
  * Input: 
  * - File name
  * - Buffer to store the read contents
  * - Debug flag
  * Output: 
  * - -1: Error
  * - 0: Success
*/
int readFile(char* fileName, char* buffer, uint8_t debugFlag) {
  // Open the file
  int fileDescriptor;
  printf("Opening file %s...\n", fileName);
  fileDescriptor = open(fileName, O_CREAT, O_RDWR); // Create if does not exist + read and write mode
  if (fileDescriptor == -1) {
    perror("Error opening file");
    return -1;
  }
  printf("File %s opened\n", fileName);

  // Get the size of the file in bytes
  struct stat fileInformation;
  if (stat(fileName, &fileInformation) == -1) {
    perror("Error getting file size");
    return -1;
  };
  unsigned long int fileSize = fileInformation.st_size;
  if (debugFlag) {
    printf("%s is %ld bytes\n", fileName, fileSize);
  }

  // Read out the contents of the file
  printf("Reading file...\n");
  ssize_t bytesReadFromFile = 0;
  bytesReadFromFile = read(fileDescriptor, buffer, fileSize);
  if (bytesReadFromFile == -1) {
    perror("Error reading file");
    return -1;
  }
  if (debugFlag) {
    printf("%zd bytes read from %s\n", bytesReadFromFile, fileName);
  }
  printf("File read successfully\n");
  return 0;
}

/*
  * Name: writeFile
  * Purpose: Open a file and write to it
  * Input: 
  * - Name of the file
  * - What to write to the file
  * - Length of data to be written to the file
  * Output:
  * - -1: Error
  * - 0: Success
*/
int writeFile(char* fileName, char* fileContents, size_t fileSize) {
  // Open file to write to
  int fileDesciptor;
  fileDesciptor = open(fileName, (O_CREAT | O_RDWR), S_IRWXU); // Create if doesn't exist. Read/write
  if (fileDesciptor == -1) {
    perror("Error opening file");
    return -1;
  }

  // Write to the new file
  int writeReturn = write(fileDesciptor, fileContents, fileSize);
  if (writeReturn == -1) {
    perror("File write error");
    return -1;
  }
  return 0;
}

/*
  * Name: fileNameFromCommand
  * Purpose: Extract the file name from a command
  * Input: 
  * - The user input/command
  * - String to return the file name in
  * Output: None
*/
void fileNameFromCommand(char* userInput, char* fileName) {
  char* tempUserInput = userInput;
  tempUserInput += 5;
  strcpy(fileName, tempUserInput);
}

