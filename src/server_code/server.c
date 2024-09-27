
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
#include <errno.h>

#include "../common/network_node.h"
#include "server.h"

// Global flags
uint8_t debugFlag = 0;  // Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
//struct addrinfo* serverAddressInfo;
int socketDescriptor;
struct sockaddr_in serverAddress;
//struct sockaddr_in clientAddress;
//int incomingSocketDescriptor;
//socklen_t clientAddressLength = sizeof(clientAddress);

// Forward declarations
void shutdownServer(int);

// Main fucntion
int main(int argc, char* argv[]) {
  // Assign callback function for Ctrl-c
  signal(SIGINT, shutdownServer);

  // Set up server sockaddr_in data structure
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(PORT);

/*
  packetFields serverPacketFields;
  serverPacketFields.delimiter = "delimFlag";
  serverPacketFields.messageBegin = "messageBegin";
  serverPacketFields.messageEnd = "messageEnd";
  serverPacketFields.putCommand = "put";
  serverPacketFields.getCommand = "get";
*/
  
  // Check how many command line arguments passed
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

  /*
	int status;
	struct addrinfo hints;

	hints.ai_family = AF_INET;        // IPV4
	hints.ai_socktype = SOCK_STREAM;  // TCP
	hints.ai_protocol = 0;            // Any protocol
	hints.ai_flags = AI_PASSIVE;      // If node is null, will bind to IP of host
	
  // Port 3940
	int getaddrinfoReturnValue;
	getaddrinfoReturnValue = getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);
	if (getaddrinfoReturnValue != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoReturnValue));	
		exit(EXIT_FAILURE);
	}
  */
  
  // Set up socket
  printf("Setting up socket...\n");
  socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketDescriptor == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Error when setting up socket: %s", errorMessage);
    exit(1);
  }
  printf("Socket set up\n");

  // Bind socket
  printf("Binding socket...\n");
  int bindReturn = bind(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindReturn == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Error when binding socket: %s", errorMessage);
  }
  printf("Socket bound\n");
  
/*	
  // Listen
  listen(socketDescriptor, 10);		// Limit queued connections to 10

  struct sockaddr incomingAddress;
  int incomingSocketDescriptor;
  socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);
*/

  //pid_t processId;

  char message[INITIAL_MESSAGE_SIZE];
  // Continously listen for new UDP packets
  while (1) {
    int n = recvfrom(socketDescriptor, message, 100, 0, 0, 0);
    if (n < 0) {
      char* errorMessage = malloc(1024);
      strcpy(errorMessage, strerror(errno));
      printf("%s\n", errorMessage);
      exit(1);
    }
    printf("Received %d bytes", n);
    if (n > 2) {
      printf("%s\n", message);
    }

 /* 
    // Accept the incoming connection
    printf("Listening for connections...\n");
    incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);
    printf("Connection accepted\n");
    printf("Listening for data...\n");
    
    if ((processId = fork()) == 0) {
      close(socketDescriptor); 
      uint8_t clientAlive = 1;

      // Process incoming data
//      char* incomingFileName = malloc(FILE_NAME_SIZE);      // Space for file name
      fcntl(incomingSocketDescriptor, F_SETFL, O_NONBLOCK); // Set socket to non blocking (will return if no data available)
      int receivePacketReturn;

      while(clientAlive) {
        char* incomingFileName = malloc(FILE_NAME_SIZE);      // Space for file name
        receivePacketReturn = receivePacket(incomingSocketDescriptor, incomingFileName, FILE_NAME_SIZE, serverPacketFields, debugFlag);
        switch (receivePacketReturn) {
          case 0: // get command
            sendPacket(incomingFileName, incomingSocketDescriptor, serverPacketFields, serverPacketFields.putCommand, debugFlag); // Send back the requested file
            break;
          case 1: // Client connection closed
            clientAlive = 0;
            break;
          default:  // put command or error (need to improve)
            break;
        }
      }

      printf("Connection terminated\n");
      exit(0);
    }
    close(incomingSocketDescriptor);
  */
  }
  return 0;
}

/*
* Name: shutdownServer
* Purpose: Gracefully shutdown the server when the user enters
* ctrl-c. Closes the sockets and frees addrinfo data structure
* Input: The signal raised
* Output: None
*/
void shutdownServer(int signal) {
//  close(incomingSocketDescriptor);
  close(socketDescriptor);
//	freeaddrinfo(serverAddressInfo);
  printf("\n");
  exit(0);
}
