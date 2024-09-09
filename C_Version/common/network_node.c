#define MAX_PACKET_LENGTH 1500

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "network_node.h"


int networkNodeConnect(const char* nodeName, int socketDescriptor, struct addrinfo* destinationAddressInfo) {
  printf("Connecting to %s...\n", nodeName);
  int connectionStatus;
  connectionStatus = connect(socketDescriptor, destinationAddressInfo->ai_addr, destinationAddressInfo->ai_addrlen);
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

char* packetAppend(char* destinationPacket, const char* sourceInformation, const char* informationName) {
  size_t sourceInformationLength = strlen(sourceInformation);
  destinationPacket = strncat(destinationPacket, sourceInformation, sourceInformationLength); // Add beginning of message
  printf("Added %zd byte %s to packet\n", sourceInformationLength, informationName);
  return destinationPacket;
}

/*
 * Name: sendFile
 * Purpose: Send a file along with its name to a waiting server
 * Input: 
 * - The name of the file to send
 * - Socket Descriptor of the socket to send the file out on
 * Output: None
 */
void sendFile(const char* fileName, int socketDescriptor, packetFields senderPacketFields, uint8_t debugFlag) {
  // Begin building the packet
  char* filePacket = malloc(MAX_PACKET_LENGTH);
  printf("Allocated %d byte packet\n", MAX_PACKET_LENGTH);
  
  // Add beginning to message to packet
  filePacket = packetAppend(filePacket, senderPacketFields.messageBegin, "beginning of message");

  // Add send command to packet
  filePacket = packetAppend(filePacket, senderPacketFields.sendCommand, "send command");

  // Add delimiter to packet
  filePacket = packetAppend(filePacket, senderPacketFields.delimiter, "delimiter");

  // Add file name to packet
  filePacket = packetAppend(filePacket, fileName, "file name");
  
  // Add delimiter to packet
  filePacket = packetAppend(filePacket, senderPacketFields.delimiter, "delimiter");

  // Add file contents to packet
  printf("Adding contents of %s to packet\n", fileName);
  // Open the file
  int fileDescriptor;
  printf("Opening file %s...\n", fileName);
  fileDescriptor = open(fileName, O_CREAT, O_RDWR);	// Create if does not exist + read and write mode
  if (fileDescriptor == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Failed to open file \"%s\" with error %s\n", fileName, errorMessage);
    exit(1);
  }
  printf("File %s opened\n", fileName);

  // Get the size of the file in bytes
  struct stat fileInformation;
  if (stat(fileName, &fileInformation) == -1) {
    printf("Stat Error\n");
    exit(1);
  };
  unsigned long int fileSize = fileInformation.st_size;
  printf("%s is %ld bytes\n", fileName, fileSize);

  // Read the contents of the file into file contents
  char* fileContents = malloc(100000);
  printf("Reading file into buffer...\n");
  ssize_t bytesReadFromFile = 0;
  bytesReadFromFile = read(fileDescriptor, fileContents , fileSize);
  if (bytesReadFromFile == -1) {            // read failed()
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Failed to read file \"%s\" with error %s\n", fileName, errorMessage);
    exit(1);
  }
  else {
    printf("%zd bytes read from %s\n", bytesReadFromFile, fileName);
  }
  
  // Add file contents to packet
  filePacket = packetAppend(filePacket, fileContents, "file contents");

  // Add message end to packet
  filePacket = packetAppend(filePacket, senderPacketFields.messageEnd, "message end");
  
  // Print out the packet that is going to be sent
  printf("%zd byte packet to be sent: %s\n", strlen(filePacket), filePacket);

  // Send the packet
  printf("Sending packet...\n");
  int bytesSent = sendBytes(socketDescriptor, filePacket, strlen(filePacket), debugFlag);
  printf("%d byte packet sent\n", bytesSent);
}

/*
 * Name: sendBytes
 * Purpose: Send a desired number of bytes out on a Socket
 * Input:
 * - Socket Descriptor of the socket to send the bytes with
 * - Buffer containing the bytes to send
 * - Amount of bytes to send
 * Output: Number of bytes sent
 * Notes: need to change variable names to be more ambiguous
 */
int sendBytes(int socketDescriptor, const char* buffer, unsigned long int bufferSize, uint8_t debugFlag) {
  if (debugFlag) {
    // Print the bytes to send
    printf("Bytes to be sent:\n\n");
    int i;
    for (i = 0; i < bufferSize; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n \n");
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
 * Name: receiveFile
 * Purpose: This function is for reading a file into the present working directory.
 * The file name will be the same as the file that was sent.
 * Input: Socket Descriptor of the accepted transmission
 * Output: None
 */
void receiveFile(int incomingSocketDescriptor, uint8_t debugFlag) {
  // Receive file
  printf("Receiving File...\n");
  char* incomingPacket = malloc(MAX_PACKET_LENGTH);
  int bytesReceived = 0;
  int totalBytesReceived = 0;
  while (bytesReceived = recv(incomingSocketDescriptor, incomingPacket, MAX_PACKET_LENGTH, 0)) {
    totalBytesReceived += bytesReceived;
  }
  printf("%d byte packet received:\n%s\n", totalBytesReceived, incomingPacket);

  // Parse incomingPacket



  /*

  int bytesReceived;
  int i;

  // Receive file name
  printf("Receiving file name...\n");
  char* receivedFileName = malloc(20);
  bytesReceived = receiveBytes(incomingSocketDescriptor, receivedFileName, 20, debugFlag);
  printf("%d byte filename received: %s\n", bytesReceived, receivedFileName);
  

  // Receive file contents
  printf("Receiving file contents...\n");
  char* fileContents = malloc(1000);
  bytesReceived = receiveBytes(incomingSocketDescriptor, fileContents, 1000, debugFlag);
  printf("Received %d bytes of file content\n", bytesReceived);

  // Open and write to the new file
  int receivedFile;
  printf("Opening received file...\n");
  receivedFile = open(receivedFileName, (O_CREAT | O_RDWR), S_IRWXU);
  printf("Received file opened\n");
  printf("Writing received file...\n");
  write(receivedFile, fileContents, bytesReceived);
  printf("Received file written\n");

  */
  printf("File received\n");
}


