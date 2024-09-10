#define FILE_NAME_SIZE 50

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

#include "../common/network_node.h"
#include "server.h"

// Global flags
uint8_t debugFlag = 0;  // Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
struct addrinfo* serverAddressInfo;
int socketDescriptor;
int incomingSocketDescriptor;

// Forward declarations
void shutdownServer(int);

// Main fucntion
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownServer);

  packetFields serverPacketFields;
  serverPacketFields.delimiter = "delimFlag";
  serverPacketFields.messageBegin = "messageBegin";
  serverPacketFields.messageEnd = "messageEnd";
  serverPacketFields.putCommand = "put";
  serverPacketFields.getCommand = "get";


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
	
  // Port 3940
	int getaddrinfoReturnValue;
	getaddrinfoReturnValue = getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);
	if (getaddrinfoReturnValue != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoReturnValue));	
		exit(EXIT_FAILURE);
	}
  
  // Set up socket
  printf("Setting up socket...\n");
  socketDescriptor = socket(serverAddressInfo->ai_family, serverAddressInfo->ai_socktype, 0);
  printf("Socket set up\n");

  // Bind socket
  printf("Binding socket...\n");
  bind(socketDescriptor, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen);
  printf("Socket bound\n");
	
  // Listen
  listen(socketDescriptor, 10);		// Limit queued connections to 10

  struct sockaddr incomingAddress;
  int incomingSocketDescriptor;
  socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);

  pid_t processId;

  // Continously listen for new packets
  while (1) {
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
    printf("here\n");
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
  close(incomingSocketDescriptor);
  close(socketDescriptor);
	freeaddrinfo(serverAddressInfo);
  printf("\n");
  exit(0);
}
