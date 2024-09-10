#define USER_INPUT_BUFFER_LENGTH 40
#define FILE_NAME_SIZE 50

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "../common/network_node.h"
#include "client.h"

// Global flags
uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
int socketDescriptor;
struct addrinfo* clientAddressInfo;

// Forward declarations
void printFileInformation(const char*, struct stat);
void shutdownClient(int);

// Main
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownClient);

  // Set packet fields
  packetFields clientPacketFields;
  clientPacketFields.delimiter = "delimFlag";
  clientPacketFields.messageBegin = "messageBegin";
  clientPacketFields.messageEnd = "messageEnd";
  clientPacketFields.putCommand = "put";
  clientPacketFields.getCommand = "get";

	switch (argc) { // Check how many command line arguments are passed
		case 1:
			printf("Running client in normal mode\n");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) { // Check if debug flag
				debugFlag = 1;
				printf("%s\n", "Running client in debug mode");
			}
			else {
				printf("Invalid usage of client");  // Could make this printout better
			}
			break;
    default:
			printf("Invalid usage of client");  // Could make this printout better
	}
	
	int status;
	struct addrinfo hints;

  // Setup hints 
	hints.ai_family = AF_INET;                              // Internet
	hints.ai_socktype = SOCK_STREAM;                        // TCP
	hints.ai_protocol = 0;                                  // Auto set based on family and socktype

	getaddrinfo(NULL, "3940", &hints, &clientAddressInfo);  // Port 3940

	socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0);

  // Connect to the server
  const char* nodeName = "server";
  socketDescriptor = networkNodeConnect(nodeName, socketDescriptor, clientAddressInfo);

  // Constantly check user input for a put/get command
  while(1) {
    // Get user input and store in userInput buffer
		char userInput[USER_INPUT_BUFFER_LENGTH];
    fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);
    userInput[strcspn(userInput, "\n")] = 0;                // Remove \n

    // put
		if (userInput[0] == 'p' && userInput[1] == 'u' && userInput[2] == 't') {
      // Send file
      sendPacket(&userInput[4], socketDescriptor, clientPacketFields, clientPacketFields.putCommand, debugFlag);  
      //close(socketDescriptor);                                                                    // Close current connection
      //socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0); // New socket descriptor for next connection
		}
    // get
		else if (userInput[0] == 'g' && userInput[1] == 'e' && userInput[2] == 't') {
      // Send get command and receive file
      sendPacket(&userInput[4], socketDescriptor, clientPacketFields, clientPacketFields.getCommand, debugFlag);  // Send get command packet
      char* incomingFileName = malloc(FILE_NAME_SIZE);  // Space for file name
      fcntl(socketDescriptor, F_SETFL, O_NONBLOCK);     // Set socket to non blocking (don't wait on data)
      receivePacket(socketDescriptor, incomingFileName, FILE_NAME_SIZE, clientPacketFields, debugFlag); // Receive file packet
      //close(socketDescriptor);                                                                          // Close current connection
      //socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0);       // New socket descriptor for next connection
      
		}
    else {
      // Enter valid command (put/get)
    }
	}
	return 0;
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

void shutdownClient(int signal) {
  close(socketDescriptor);
	freeaddrinfo(clientAddressInfo);
  printf("\n");
  exit(0);
}
