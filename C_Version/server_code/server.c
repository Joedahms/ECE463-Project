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
uint8_t debugFlag = 0; // Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
struct addrinfo *serverAddressInfo;
int socketDescriptor;
int incomingSocketDescriptor;

// Forward declarations
void shutdownServer(int);

// Main fucntion
int main(int argc, char *argv[])
{
  signal(SIGINT, shutdownServer);

  // Check how many command line areguements passed
  switch (argc)
  {
  case 1:
    printf("Running server in normal mode\n");
    break;
  case 2:
    if (strcmp(argv[1], "-d") == 0)
    {
      debugFlag = 1;
      printf("Running server in debug mode\n");
    }
    break;
  default:
  }

  int status;
  struct addrinfo hints;

  hints.ai_family = AF_INET;       // IPV4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_protocol = 0;           // Any protocol
  hints.ai_flags = AI_PASSIVE;     // If node is null, will bind to IP of host

  int getaddrinfoReturnValue;
  getaddrinfoReturnValue = getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);
  if (getaddrinfoReturnValue != 0)
  {
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

  listen(socketDescriptor, 10); // Limit queued connections to 10
  printf("Listening...\n");

  struct sockaddr incomingAddress;
  int incomingSocketDescriptor;
  socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);

  // Continuously listen for new files
  while (1)
  {
    incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress);
    printf("Connection accepted\n");

    // Receive the command
    printf("Receiving command...\n");
    int commandSize = 0;
    char *commandBuffer = malloc(4);
    commandSize = receiveBytes(incomingSocketDescriptor, commandBuffer, 3, debugFlag);
    printf("Received %d byte command of type %s\n", commandSize, commandBuffer);

    // Receive a file
    if (strcmp("put", commandBuffer) == 0)
    {
      receiveFile(incomingSocketDescriptor, debugFlag);
      close(incomingSocketDescriptor);
      printf("Connection terminated\n");
    }

    // Send a file
    else if (strcmp("get", commandBuffer) == 0)
    {
      char *requestedFileName = malloc(20);
      int fileNameBytesReceived;
      fileNameBytesReceived = receiveBytes(incomingSocketDescriptor, requestedFileName, 20, debugFlag);
      printf("%d byte filename received: %s\n", fileNameBytesReceived, requestedFileName);

      sendFile(requestedFileName, incomingSocketDescriptor, debugFlag);
      close(incomingSocketDescriptor);
      printf("Connection terminated\n");
    }

    // Invalid command
    else
    {
    }
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
void shutdownServer(int signal)
{
  close(incomingSocketDescriptor);
  close(socketDescriptor);
  freeaddrinfo(serverAddressInfo);
  printf("\n");
  exit(0);
}
