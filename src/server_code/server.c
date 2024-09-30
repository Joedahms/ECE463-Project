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
int listeningUDPSocketDescriptor;
int listeningTCPSocketDescriptor;
int connectedTCPSocketDescriptor;

struct connectedClient connectedClients[MAX_CONNECTED_CLIENTS];

// Main fucntion
int main(int argc, char* argv[]) {
  // Assign callback function for Ctrl-c
  signal(SIGINT, shutdownServer);

  // Array of connected client data structures
  int i; 
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
    memset(&(connectedClients[i].socketAddress), 0, sizeof(connectedClients[i].socketAddress));
  }

  struct sockaddr_in serverAddress;
  struct sockaddr_in clientUDPAddress;
  struct sockaddr_in clientTCPAddress;

  // Set up server sockaddr_in data structure
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(PORT);
  
  checkCommandLineArguments(argc, argv, &debugFlag);

  setupUdpSocket(serverAddress);
  setupTcpSocket(serverAddress);
  
  char message[INITIAL_MESSAGE_SIZE];
  
  int udpStatus;
  int tcpStatus;
  // Continously listen for new UDP packets
  int j = 0;
  while (1) {
    udpStatus = checkUdpSocket(message, debugFlag);
    switch (udpStatus) {
      case 0:   // Nothing
        break;

      case 1:   // Plain text
        break;

      case 2:   // Put command
        break;

      case 3:   // Get command
        break;

      case 4:   // Invalid command
        break;

      default:
        
    }
    tcpStatus = checkTcpSocket(&clientTCPAddress, debugFlag);
    if (tcpStatus == 0) { // No data to be read
      ; 
    }
    else {                // Data to be read
      if (debugFlag) {
        printf("main port: %d\n", ntohs(clientTCPAddress.sin_port));
      }

      handleTcpConnection(clientTCPAddress, debugFlag);

      if (debugFlag) {
        printf("cc0 port: %d\n", ntohs(connectedClients[0].socketAddress.sin_port));
        printf("cc0 address: %d\n\n", connectedClients[0].socketAddress.sin_addr.s_addr);

        printf("cc1 port: %d\n", ntohs(connectedClients[1].socketAddress.sin_port));
        printf("cc1 address: %d\n\n", connectedClients[1].socketAddress.sin_addr.s_addr);
      }
    }
  }

      /*
      {
        if (strncmp(&message[1], "put", 3) == 0) {        // %put
          printf("Received put command\n");
          printf("Forking new process\n");

            int totalBytesReceived = 0;
            char* buffer = malloc(100);


            while(clientConnected) {
              while (totalBytesReceived = recv(connectedTCPSocketDescriptor, buffer, 100, 0)) { // Constantly check the socket for data
                if (debugFlag) {
                  printf("%d\n", totalBytesReceived);
                }
                if (totalBytesReceived != -1) { // Something was actually received or client closed connection
                  break;
                }
              }
              
              if (totalBytesReceived == 0) {
                printf("Client connection closed\n");
                clientConnected = 0;
              }
              else {
                printf("%s\n", buffer);
              }
            }
        }
        else if (strncmp(&message[1], "get", 3) == 0) {
          printf("Received get command\n");
        }
        else {                                            // Invalid command
          printf("Received invalid command\n");
        }
        exit(0);
      }
      */
    /*
    else {                                              // Message was plain text
      printf("Message received was a plain text message\n");
    }
    memset(&message[0], 0, INITIAL_MESSAGE_SIZE);       // Clear out message buffer
    */

    
   

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
  close(listeningUDPSocketDescriptor);
  close(listeningTCPSocketDescriptor);
  close(connectedTCPSocketDescriptor);
  printf("\n");
  exit(0);
}

/*
  * Name: handleErrorNonBlocking
  * Purpose: Check the return after checking a non blocking socket
  * Input: The return value from checking the socket
  * Output:
  * - 0: There is data waiting to be read
  * - 1: No data waiting to be read
*/

int handleErrorNonBlocking(int returnValue) {
  if (returnValue == -1) {                          // Error
    if (errno == EAGAIN || errno == EWOULDBLOCK) {  // Errors occuring from no message on non blocking socket
      return 1;
    }
    else {                                          // Relevant error
      perror("Error when checking non blocking socket");
      exit(1);
      return 1;
    }
  }
  else {                                            // Got a message
    return 0;
  }
}

void setupUdpSocket(struct sockaddr_in serverAddress) {
  // Set up UDP socket
  printf("Setting up UDP socket...\n");
  listeningUDPSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (listeningUDPSocketDescriptor == -1) {
    perror("Error when setting up UDP socket");
    exit(1);
  }
  fcntl(listeningUDPSocketDescriptor, F_SETFL, O_NONBLOCK);
  printf("UDP socket set up\n");

  // Bind UDP socket
  printf("Binding UDP socket...\n");
  int bindReturnUDP = bind(listeningUDPSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindReturnUDP == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Error when binding UDP socket: %s\n", errorMessage);
    exit(1);
  }
  printf("UDP socket bound\n");
}

void setupTcpSocket(struct sockaddr_in serverAddress) {
  // Set up TCP socket
  printf("Setting up TCP socket...\n");
  listeningTCPSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (listeningTCPSocketDescriptor == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Error when setting up TCP socket: %s", errorMessage);
    exit(1);
  }
  fcntl(listeningTCPSocketDescriptor, F_SETFL, O_NONBLOCK);
  printf("TCP socket set up\n");

  // Bind TCP socket
  printf("Binding TCP socket...\n");
  int bindReturnTCP = bind(listeningTCPSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindReturnTCP == -1) {
    char* errorMessage = malloc(1024);
    strcpy(errorMessage, strerror(errno));
    printf("Error when binding TCP socket: %s\n", errorMessage);
    exit(1);
  }
  printf("TCP socket bound\n");

  // Set socket to listen
  int listenReturn = listen(listeningTCPSocketDescriptor, 10);
  if (listenReturn == -1) {
    perror("TCP socket listen error");
    exit(1);
  }
}

// Check to see if any messages queued at UDP socket
int checkUdpSocket(char* message, uint8_t debugFlag) {
  int bytesReceived = recvfrom(listeningUDPSocketDescriptor, message, INITIAL_MESSAGE_SIZE, 0, 0, 0);
  int nonBlockingReturn = handleErrorNonBlocking(bytesReceived);

  if (nonBlockingReturn == 1) {                 // No incoming packets
    return 0;
  }
  
  // Print out UDP message
  printReceivedMessage(bytesReceived, message, debugFlag); 

  if (checkStringForCommand(message) == 0) {    // Message is plain text
    return 1;                                   // return 1
  }
  else if (strncmp(message, "%put ", 5) == 0) { // Received put command
    printf("Received put command\n"); 
    return 2;                                   // Return 2
  }
  else if (strncmp(message, "%get ", 5) == 0) { // Received get command
    printf("Received get command\n");
    return 3;                                   // Return 3
  }
  else {                                        // Received invalid command
    printf("Received invalid command\n");
    return 4;                                   // Return 4
  }
}

// Check to see if any incoming TCP connections from new clients
int checkTcpSocket(struct sockaddr_in* incomingAddress, uint8_t debugFlag) {
  int fd[2];
  int fd2[2];
  pid_t processId;

  socklen_t incomingAddressLength = sizeof(incomingAddress);
  connectedTCPSocketDescriptor = accept(listeningTCPSocketDescriptor, (struct sockaddr *)incomingAddress, &incomingAddressLength);
  int returnCheck = connectedTCPSocketDescriptor;
  int nonBlockingReturn = handleErrorNonBlocking(returnCheck);
  if (nonBlockingReturn == 0) {           // Data to be read
    return 1;                             // Return 1
  }
  else {                                  // No data to be read
    return 0;                             // Return 0
  }
}

void handleTcpConnection(struct sockaddr_in clientTCPAddress, uint8_t debugFlag) {
  if (debugFlag) {
    printf("clientTCPAddress port at start of handleTCPConnection: %d\n", ntohs(clientTCPAddress.sin_port));  
  }
 
  // Find available space for new client
  int availableConnectedClient = findEmptyConnectedClient(debugFlag); 
  if (availableConnectedClient == -1) {
    printf("Error: server cannot handle any additional clients");
    exit(1);
  }
  if (debugFlag) {
    printf("availableConnectedClient: %d\n", availableConnectedClient);
  }

  // Initialize the new client
  connectedClients[availableConnectedClient].socketAddress = clientTCPAddress;
  connectedClients[availableConnectedClient].serverParentToChildPipe;
  connectedClients[availableConnectedClient].serverChildToParentPipe;

  // Setup pipes
  pipe(connectedClients[0].serverParentToChildPipe);
  pipe(connectedClients[0].serverChildToParentPipe);

  // Fork a new process for the client
  pid_t processId;
  if ((processId = fork()) == -1) {                         // Fork error
    perror("Fork error"); 
  }
  else if (processId == 0) {                                // Child process
    close(connectedClients[0].serverParentToChildPipe[1]);  // Close write on parent -> child.
    close(connectedClients[0].serverChildToParentPipe[0]);  // Close read on child -> parent.

    // char* dataFromParent = malloc(100);
    // char* test = malloc(100);
    char dataFromParent[100];
    char test[100];
    int bytesFromParent = 0;

    uint8_t clientConnected = 1;
    int i;
    for(i = 0; i < 10; i++) {
      // Check for command from parent thru pipe
      bytesFromParent = read(connectedClients[0].serverParentToChildPipe[0], dataFromParent, sizeof(dataFromParent));
      printf("%s\n", dataFromParent);
    }
    exit(0);
  }
  else {                                                    // Parent process
    if (debugFlag) {
      printf("Parent process id?: %d\n", processId);
    }

    close(connectedClients[0].serverParentToChildPipe[0]);  // Close read on parent -> child. Write on this pipe
    close(connectedClients[0].serverChildToParentPipe[1]);  // Close write on child -> parent. Read on this pipe

    write(connectedClients[0].serverParentToChildPipe[1], "hello", 6);
  }
}

int findEmptyConnectedClient(uint8_t debugFlag) {
  int i;
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
    int port = ntohs(connectedClients[i].socketAddress.sin_port);
    if (port == 0) {
      return i;
      if (debugFlag) {
        printf("%d is empty\n", i);
      }
    }
    else {
      if (debugFlag) {
        printf("%d is not empty\n", i);
      }
    }
  }
  return -1;
}
