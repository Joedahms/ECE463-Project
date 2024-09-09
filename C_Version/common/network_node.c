#define MAX_PACKET_LENGTH 1500 // Upper limit on packet size (bytes)

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <sys/socket.h>
#include "network_node.h"

int networkNodeConnect(const char *nodeName, int socketDescriptor, struct addrinfo *destinationAddressInfo)
{
  printf("Connecting to %s...\n", nodeName);
  int connectionStatus;
  connectionStatus = connect(socketDescriptor, destinationAddressInfo->ai_addr, destinationAddressInfo->ai_addrlen);
  // Check if connection was successful
  if (connectionStatus != 0)
  {
    char *errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Connection to %s failed with error %s\n", nodeName, errorMessage);
    exit(1);
  }
  printf("Connected to %s...\n", nodeName);
  return socketDescriptor;
}

/*
 * Name: packetAppend
 * Purpose: Append a string to a packet and print confirmation
 * Input:
 * - Packet to append string to
 * - String to append to packet
 * - What is being added to the packet
 * Output: Packet with the new string appended to it
 */
char *packetAppend(char *destinationPacket, const char *sourceInformation, const char *informationName)
{
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
 * - Structure containing packet packet fields
 * - Which command to send (put or get)
 * - debug flag
 * Output: None
 */
<<<<<<< HEAD
void sendFile(const char *fileName, int socketDescriptor, uint8_t debugFlag)
{
  // Send the file name
  int fileNameLength = strlen(fileName);
  printf("Sending file: %s\n", fileName);
  printf("Sending %d byte filename: %s...\n", fileNameLength, fileName);
  int bytesSent = 0;
  bytesSent = sendBytes(socketDescriptor, fileName, fileNameLength, debugFlag);
  printf("Sent %d bytes\n", bytesSent);

  // Open the file
  int fileDescriptor;
  printf("Opening file...\n");
  fileDescriptor = open(fileName, O_CREAT, O_RDWR); // Create if does not exist + read and write mode
  if (fileDescriptor == -1)
  {
    char *errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Failed to open file \"%s\" with error %s\n", fileName, errorMessage);
    exit(1);
  }
  printf("File Open\n");

  // Get the size of the file in bytes
  struct stat fileInformation;
  if (stat(fileName, &fileInformation) == -1)
  {
    printf("Stat Error\n");
    exit(1);
  };
  unsigned long int fileSize = fileInformation.st_size;
  printf("%s is %ld bytes\n", fileName, fileSize);

  // Read the contents of the file into the file buffer
  char *fileBuffer = malloc(100000);
  printf("Reading file into buffer...\n");
  ssize_t bytesReadFromFile = 0;
  bytesReadFromFile = read(fileDescriptor, fileBuffer, fileSize);
  if (bytesReadFromFile == -1)
  { // read failed()
    char *errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Failed to read file \"%s\" with error %s\n", fileName, errorMessage);
    exit(1);
  }
  else
  {
    printf("%zd bytes read from %s\n", bytesReadFromFile, fileName);

    void sendPacket(const char *fileName, int socketDescriptor, packetFields senderPacketFields, char *command, uint8_t debugFlag)
    {
      char *packet = malloc(MAX_PACKET_LENGTH); // Allocate memory for packet
      printf("Allocated %d byte packet\n", MAX_PACKET_LENGTH);

      // Add beginning of message to packet
      packet = packetAppend(packet, senderPacketFields.messageBegin, "beginning of message");

      // Add command to packet
      uint8_t putCommandFlag = 0;
      // put
      if (strcmp(senderPacketFields.putCommand, command) == 0)
      {
        putCommandFlag = 1;
        packet = packetAppend(packet, senderPacketFields.putCommand, "put command");
      }
      // get
      else if (strcmp(senderPacketFields.getCommand, command) == 0)
      {
        packet = packetAppend(packet, senderPacketFields.getCommand, "get command");
      }
      else
      {
        printf("Invalid command passed to sendPacket()\n");
        exit(1);
>>>>>>> main
      }

      // Add delimiter to packet
      packet = packetAppend(packet, senderPacketFields.delimiter, "delimiter");

      // Add file name to packet
      packet = packetAppend(packet, fileName, "file name");

      // Add delimiter to packet
      packet = packetAppend(packet, senderPacketFields.delimiter, "delimiter");

      // If file contents are to be sent
      if (putCommandFlag)
      {
        // Add file contents to packet
        printf("Adding contents of %s to packet\n", fileName);
        // Open the file
        int fileDescriptor;
        printf("Opening file %s...\n", fileName);
        fileDescriptor = open(fileName, O_CREAT, O_RDWR); // Create if does not exist + read and write mode
        if (fileDescriptor == -1)
        {
          char *errorMessage = malloc(1024);
          strcpy(errorMessage, strerror(errno));
          printf("Failed to open file \"%s\" with error %s\n", fileName, errorMessage);
          exit(1);
        }
        printf("File %s opened\n", fileName);

        // Get the size of the file in bytes
        struct stat fileInformation;
        if (stat(fileName, &fileInformation) == -1)
        {
          printf("Stat Error\n");
          exit(1);
        };
        unsigned long int fileSize = fileInformation.st_size;
        printf("%s is %ld bytes\n", fileName, fileSize);

        // Read out the contents of the file
        char *fileContents = malloc(100000);
        printf("Reading file...\n");
        ssize_t bytesReadFromFile = 0;
        bytesReadFromFile = read(fileDescriptor, fileContents, fileSize);
        if (bytesReadFromFile == -1)
        { // read failed()
          char *errorMessage = malloc(1024);
          strcpy(errorMessage, strerror(errno));
          printf("Failed to read file \"%s\" with error %s\n", fileName, errorMessage);
          exit(1);
        }
        else
        {
          printf("%zd bytes read from %s\n", bytesReadFromFile, fileName);
        }

        // Add file contents to packet
        packet = packetAppend(packet, fileContents, "file contents");
      }
      // get
      else
      {
        char *dummyFileContents = malloc(1); // No contents sent with a get command
        packet = packetAppend(packet, dummyFileContents, "dummy file contents");
      }

      // Add message end to packet
      packet = packetAppend(packet, senderPacketFields.messageEnd, "message end");

      // Print out the packet that is going to be sent
      printf("%zd byte packet to be sent: %s\n", strlen(packet), packet);

      // Send the packet
      printf("Sending packet...\n");
      int bytesSent = sendBytes(socketDescriptor, packet, strlen(packet), debugFlag);
      printf("%d byte packet sent\n", bytesSent);
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
     * Notes: need to change variable names to be more ambiguous
     */
    int sendBytes(int socketDescriptor, const char *buffer, unsigned long int bufferSize, uint8_t debugFlag)
    {
      if (debugFlag)
      {
        // Print the bytes to send
        printf("Bytes to be sent:\n\n");
        int i;
        for (i = 0; i < bufferSize; i++)
        {
          printf("%c", buffer[i]);
        }
        printf("\n \n");
      }

      int bytesSent = 0;
      bytesSent = send(socketDescriptor, buffer, bufferSize, 0);
      if (bytesSent == -1)
      {
        char *errorMessage = malloc(1024);
        strcpy(errorMessage, strerror(errno));
        printf("Byte send failed with error %s\n", errorMessage);
        exit(1);
      }
      else
      {
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
    int receiveBytes(int incomingSocketDescriptor, char *buffer, int bufferSize, uint8_t debugFlag)
    {
      int numberOfBytesReceived = 0;
      numberOfBytesReceived = recv(incomingSocketDescriptor, buffer, bufferSize, 0);
      if (debugFlag)
      {
        // Print out incoming message
        int i;
        printf("Bytes received: \n");
        for (i = 0; i < numberOfBytesReceived; i++)
        {
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
<<<<<<< HEAD
    void receiveFile(int incomingSocketDescriptor, uint8_t debugFlag)
    {
      printf("Receiving File...\n");
      == == == =
                   int receivePacket(int incomingSocketDescriptor, char *fileName, int fileNameSize, packetFields receiverPacketFields, uint8_t debugFlag)
      {
        printf("Receiving Packet...\n");
        char *incomingPacket = malloc(MAX_PACKET_LENGTH); // Allocate space for incoming packet
        int totalBytesReceived = 0;
        while (totalBytesReceived = recv(incomingSocketDescriptor, incomingPacket, MAX_PACKET_LENGTH, 0))
        { // Constantly check the socket for data
          if (debugFlag)
          {
            printf("%d\n", totalBytesReceived);
          }
          if (totalBytesReceived != -1)
          { // Something was actually received
            break;
          }
        }
        printf("%d byte packet received:\n%s\n", totalBytesReceived, incomingPacket);
>>>>>>> main

        printf("Parsing packet...\n");

<<<<<<< HEAD
        // Receive file name
        printf("Receiving file name...\n");
        char *receivedFileName = malloc(20);
        bytesReceived = receiveBytes(incomingSocketDescriptor, receivedFileName, 20, debugFlag);
        printf("%d byte filename received: %s\n", bytesReceived, receivedFileName);

        // Receive file contents
        printf("Receiving file contents...\n");
        char *fileContents = malloc(1000);
        bytesReceived = receiveBytes(incomingSocketDescriptor, fileContents, 1000, debugFlag);
        printf("Received %d bytes of file content\n", bytesReceived);
        == == == =
                     // Look for beginning of message
            printf("Checking for beginning of message...\n");
        if (strstr(incomingPacket, receiverPacketFields.messageBegin))
        { // Check if beginning exists
          printf("Beginning of message found\n");
          incomingPacket += strlen(receiverPacketFields.messageBegin); // Advance to after beginning
        }
        else
        { // Beginning of messsage not found
          printf("Invalid packet: Beginning of message not found\n");
          return 10; // change
        }

        // Check command
        uint8_t getFlag = 0; // Whether or not get has been received
        printf("Checking command...\n");
        int commandSize = 3; // Both commands are 3 bytes
        char *incomingCommand = malloc(commandSize);
        incomingCommand = strncpy(incomingCommand, incomingPacket, commandSize); // Read the command off the packet
        // put
        if (strcmp(incomingCommand, "put") == 0)
        {
          printf("Command found: put\n");
          // Care about file contents
        }
        // get
        else if (strcmp(incomingCommand, "get") == 0)
        {
          printf("Command found: get\n");
          getFlag = 1; // Dont care about the file contents
        }
        // invalid
        else
        {
          printf("Invalid command received\n");
          return 10; // change
        }
>>>>>>> main

        // Need to improve This
        // Assuming all is well and skipping delimiter to file name
        incomingPacket += 12;

        // Find file name
        printf("Checking file name...\n");
        char *incomingFileName = malloc(fileNameSize);
        char *nextPacketChar = malloc(1); // So can use strcat()
        while (strstr(incomingFileName, receiverPacketFields.delimiter) == NULL)
        { // while delimiter not found
          if (*incomingPacket == '\0')
          { // Got to end of packet before delimiter was found
            printf("filename error\n");
            return 10; // change
          }
          // Add next packet character to file name
          nextPacketChar[0] = *incomingPacket;
          incomingFileName = strcat(incomingFileName, nextPacketChar);
          incomingPacket++;
        }
        incomingFileName[strlen(incomingFileName) - 9] = '\0'; // Remove delimiter from file name
        printf("File name:\n%s\n", incomingFileName);

        // get
        if (getFlag)
        { // Dont care about file contents, just return
          strcpy(fileName, incomingFileName);
          return 0;
        }
        // put
        else
        { // Care about file contents, extract them
          // Find file contents
          printf("Checking file contents...\n");
          char *incomingFileContents = malloc(10000);
          while (strstr(incomingFileName, receiverPacketFields.messageEnd) == NULL)
          { // While end of message has not been encountered
            if (*incomingPacket == '\0')
            { // At the end of the packet
              nextPacketChar[0] = *incomingPacket;
              incomingFileContents = strcat(incomingFileContents, nextPacketChar);
              break;
            }
            // Not at the end of the packet
            nextPacketChar[0] = *incomingPacket;
            incomingFileContents = strcat(incomingFileContents, nextPacketChar); // Add the next character in the packet to the file contents
            incomingPacket++;
          }
          int i;
          for (i = 0; i < 10; i++)
          {
            incomingFileContents[strlen(incomingFileContents) - 1] = '\0';
          }
          printf("File contents:\n%s\n", incomingFileContents);

          // Open and write to the new file
          int receivedFile;
          printf("Opening received file...\n");
          receivedFile = open(incomingFileName, (O_CREAT | O_RDWR), S_IRWXU);
          printf("Received file opened\n");
          printf("Writing received file...\n");
          write(receivedFile, incomingFileContents, totalBytesReceived);
          printf("Received file written\n");
        }
        return 1;
      }
