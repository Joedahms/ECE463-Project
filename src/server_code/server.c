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

// Array of connected client data structures
struct connectedClient connectedClients[MAX_CONNECTED_CLIENTS];

// Main fucntion
int main(int argc, char* argv[]) {
  // Assign callback function for Ctrl-c
  signal(SIGINT, shutdownServer);

  // Make sure all connectedClients are set to 0
  int i; 
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
    memset(&(connectedClients[i].socketTcpAddress), 0, sizeof(connectedClients[i].socketTcpAddress));
  }

  // Initialize socket address stuctures
  struct sockaddr_in serverAddress;                   // Socket address that clients should connect to
                                                      // Same port is used for both UDP and TCP connections
  struct sockaddr_in clientUDPAddress;                // Client's UDP info
  struct sockaddr_in clientTCPAddress;                // Client's TCP info

  // Set up server sockaddr_in data structure
  memset(&serverAddress, 0, sizeof(serverAddress));   // 0 out
  serverAddress.sin_family = AF_INET;                 // IPV4
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  
  serverAddress.sin_port = htons(PORT);               // Port
  
  checkCommandLineArguments(argc, argv, &debugFlag);  // Check if user passed any arguments

  setupUdpSocket(serverAddress);                      // Setup the UDP socket
  setupTcpSocket(serverAddress);                      // Setup the TCP socket
  
  char* message = malloc(INITIAL_MESSAGE_SIZE);       // Space for incoming messages
  
  // Whether or not data is available at the socket. If it is, what kind.
  int udpStatus;
  int tcpStatus;

  // Continously listen for new UDP packets and new TCP connections
  while (1) {
    udpStatus = checkUdpSocket(listeningUDPSocketDescriptor, &clientUDPAddress, message, debugFlag);  // Check the UDP socket
    switch (udpStatus) {
      case 0:                                         // Nothing
        break;

      case 1:                                         // TCP/UDP relation info
                                                      // Message contains TCP address and port
        message += 9;                                 // Discard $address=
        char* tcpAddressString = malloc(20);          // Space for address string
        strncpy(tcpAddressString, message, 10);       // Store the address
        message += 16;                                // Discard the address and $port=
        char* tcpPortString = malloc(20);             // Space for port string
        strncpy(tcpPortString, message, 5);           // Store the port
        memset(message, 0, USER_INPUT_BUFFER_LENGTH); // Clear message for next transmission

        // Convert the address and port to integers
        long tcpAddressInteger = strtol(tcpAddressString, NULL, 10);  // Address
        long tcpPortInteger = strtol(tcpPortString, NULL, 10);        // Port

        // Find the connected client with the sent TCP address and port
        // Then assign the UDP address and port it was sent to said connected client
        int i; 
        for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) { // Loop through all connected clients
          unsigned long connectedClientAddress = ntohl(connectedClients[i].socketTcpAddress.sin_addr.s_addr); // Connected client address
          unsigned short connectedClientPort = ntohs(connectedClients[i].socketTcpAddress.sin_port);          // Connected client port

          // Does what was sent match the current connected client?
          if (tcpAddressInteger == connectedClientAddress && tcpPortInteger == connectedClientPort) {
            connectedClients[i].socketUdpAddress.sin_addr.s_addr = clientUDPAddress.sin_addr.s_addr;          // Set UDP address
            connectedClients[i].socketUdpAddress.sin_port = clientUDPAddress.sin_port;                        // Set UDP port
            break;                                                                                            // Don't loop through the rest of the connected clients
          }
        }
        if (debugFlag) {
          printAllConnectedClients(); 
        }
        break;  // Break case 1
      
      case 2:   // Plain text message
        broadcastMessage(listeningUDPSocketDescriptor, message, &clientUDPAddress);
        break;

      case 3:   // Put command
        sendCommandToChild(message, clientUDPAddress);
        break;

      case 4:   // Get command
        sendCommandToChild(message, clientUDPAddress);
        break;

      case 5:   // Invalid command
        break;

      default:  // Invalid message received
    }

    tcpStatus = checkTcpSocket(&clientTCPAddress, debugFlag); // Check for any new TCP connections
    if (tcpStatus == 0) {                                     // No new TCP connections
      ;                                                       // Do nothing
    }
    else {                                                    // New TCP connection
      handleTcpConnection(clientTCPAddress, debugFlag);       // Handle it
    }
  } // while(1)
  return 0;
} // main

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
 * Name: setupUdpSocket
 * Purpose: Setup the UDP socket. Set it to non blocking. Bind it. 
 * Input: Address structure to bind to.
 * Output: None
*/
void setupUdpSocket(struct sockaddr_in serverAddress) {
  // Set up UDP socket
  printf("Setting up UDP socket...\n");
  listeningUDPSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (listeningUDPSocketDescriptor == -1) {
    perror("Error when setting up UDP socket");
    exit(1);
  }

  // Set non blocking
  int fcntlReturn = fcntl(listeningUDPSocketDescriptor, F_SETFL, O_NONBLOCK); // Set to non blocking
  if (fcntlReturn == -1) {
    perror("Error when setting UDP socket non blocking");
  }
  printf("UDP socket set up\n");

  // Bind UDP socket
  printf("Binding UDP socket...\n");
  int bindReturnUDP = bind(listeningUDPSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); // Bind
  if (bindReturnUDP == -1) {
    perror("Error when binding UDP socket");
    exit(1);
  }
  printf("UDP socket bound\n");
}

/*
 * Name: setupTcpSocket
 * Purpose: Setup the TCP socket. Set it non blocking. Bind it. Set it to listen.
 * Input: Address structure to bind to.
 * Output: None
*/
void setupTcpSocket(struct sockaddr_in serverAddress) {
  // Set up TCP socket
  printf("Setting up TCP socket...\n");
  listeningTCPSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (listeningTCPSocketDescriptor == -1) {
    perror("Error when setting up TCP socket");
    exit(1);
  }

  // Set non blocking
  int fcntlReturn = fcntl(listeningTCPSocketDescriptor, F_SETFL, O_NONBLOCK);
  if (fcntlReturn == -1) {
    perror("Error when setting TCP socket to non blocking");
  }
  printf("TCP socket set up\n");

  // Bind TCP socket
  printf("Binding TCP socket...\n");
  int bindReturnTCP = bind(listeningTCPSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  if (bindReturnTCP == -1) {
    perror("Error when binding TCP socket");
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

/*
  * Name: checkTcpSocket
  * Purpose: Check if any new clients are trying to establish their TCP connection
  * Input:
  * - Data structure to store client info in if there is an incoming connection
  * - Debug flag
  * Output: 
  * - 0: No incoming TCP connections
  * - 1: There is an incoming connection. Its info has been stored in incomingAddress.
*/
int checkTcpSocket(struct sockaddr_in* incomingAddress, uint8_t debugFlag) {
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

/*
  * Name: handleTcpConnection
  * Purpose: If there is a new client connecting to the server, store its data, and fork a new process
  * for handling its TCP connection. Add the client to the connectedClients array. Add its data to the 
  * connectedClients array. Setup pipes so the forked child process and the parent process can communicate
  * back and forth.
  * Input: 
  * - Socket address structure of the connecting client's TCP socket
  * - Debug flag
  * Output: 
  * - None
*/
void handleTcpConnection(struct sockaddr_in clientTCPAddress, uint8_t debugFlag) {
  // Find available space in the connectedClients array for the new client
  int availableConnectedClient = findEmptyConnectedClient(debugFlag); // Find space
  if (availableConnectedClient == -1) {                               // No space
    printf("Error: server cannot handle any additional clients");
    exit(1);
  }
  if (debugFlag) {
    printf("availableConnectedClient: %d\n", availableConnectedClient);
  }

  // Initialize the new client
  connectedClients[availableConnectedClient].socketTcpAddress = clientTCPAddress; // TCP socket address structure
  connectedClients[availableConnectedClient].serverParentToChildPipe;             // Parent to child pipe
  connectedClients[availableConnectedClient].serverChildToParentPipe;             // Child to parent pipe

  // Setup pipes
  pipe(connectedClients[availableConnectedClient].serverParentToChildPipe);       // Parent to child pipe
  pipe(connectedClients[availableConnectedClient].serverChildToParentPipe);       // Child to parent pipe

  // Fork a new process for the client
  pid_t processId;
  if ((processId = fork()) == -1) { // Fork error
    perror("Error when forking a process for a new client"); 
  }
  else if (processId == 0) {        // Child process
    close(connectedClients[availableConnectedClient].serverParentToChildPipe[1]);  // Close write on parent -> child.
    close(connectedClients[availableConnectedClient].serverChildToParentPipe[0]);  // Close read on child -> parent.

    char* dataFromParent = malloc(100); // Space for data coming sent from parent
    int bytesFromParent = 0;

    uint8_t clientConnected = 1;
    int i;

    while (1) {
      // Check for command from parent thru pipe
      //bytesFromParent = read(connectedClients[availableConnectedClient].serverParentToChildPipe[0], dataFromParent, sizeof(dataFromParent));
      bytesFromParent = read(connectedClients[availableConnectedClient].serverParentToChildPipe[0], dataFromParent, 100);
      char* fileContents = malloc(MAX_FILE_SIZE);                         // Space for file contents

      if (strncmp(dataFromParent, "%get ", 5) == 0) {                     // If received a get command
        char* fileName = malloc(FILE_NAME_SIZE);                          // Space for the file name
        fileNameFromCommand(dataFromParent, fileName);                    // Extract the file name from the sent command

        int readFileReturn = readFile(fileName, fileContents, debugFlag); // Read the contents of the file into the buffer
        if (readFileReturn != -1) {
          // Send the file contents
          printf("Sending file contents...\n");
          int bytesSent = sendBytes(connectedTCPSocketDescriptor, fileContents, strlen(fileContents), debugFlag);

          if (debugFlag) {
            printf("%d byte file sent\n", bytesSent);
          }
          else {
            printf("File contents sent\n");
          }
        }
        else {
          printf("Error reading file for get request\n");
        }
      }       // End if (get)
      else if (strncmp(dataFromParent, "%put ", 5) == 0) {                // If receive put command
        char* fileName = malloc(FILE_NAME_SIZE);                          // Space for the file name
        fileNameFromCommand(dataFromParent, fileName);                    // Extract the file name from the sent command

        char* fileContents = malloc(MAX_FILE_SIZE);

        int bytesReceived = receiveBytes(connectedTCPSocketDescriptor, fileContents, MAX_FILE_SIZE, debugFlag);
        int writeFileReturn = writeFile(fileName, fileContents, strlen(fileContents));
      }
      else {                                                              // Plain text message
        sendUdpMessage(listeningUDPSocketDescriptor, connectedClients[availableConnectedClient].socketUdpAddress, dataFromParent, debugFlag); 
      }
    }         // End while(1)
    exit(0);
  }           // End child process
  else {      // Parent process
    if (debugFlag) {
      printf("Forked child PID: %d\n", processId);
    }
    connectedClients[availableConnectedClient].processId = processId;             // Set process ID

    close(connectedClients[availableConnectedClient].serverParentToChildPipe[0]); // Close read on parent -> child. Write on this pipe
    close(connectedClients[availableConnectedClient].serverChildToParentPipe[1]); // Close write on child -> parent. Read on this pipe
  }
}

/*
  * Name: findEmptyConnectedClient
  * Purpose: Loop through the connectedClients array until an empty spot is found
  * Input: debugFlag
  * Output: Index of first empty spot
*/
int findEmptyConnectedClient(uint8_t debugFlag) {
  int connectedClientsIndex;
  for (connectedClientsIndex = 0; connectedClientsIndex < MAX_CONNECTED_CLIENTS; connectedClientsIndex++) { // Loop through all connected clients
    int port = ntohs(connectedClients[connectedClientsIndex].socketTcpAddress.sin_port);  // Check if the port has been set
    if (port == 0) {                        // Port not set
      return connectedClientsIndex;         // Empty spot, return index
      if (debugFlag) {
        printf("%d is empty\n", connectedClientsIndex);
      }
    }
    else {
      if (debugFlag) {
        printf("%d is not empty\n", connectedClientsIndex);
      }
    }
  }
  return -1;                                // All spots filled
}

/*
  * Name: printAllConnectedClients
  * Purpose: Print all the connected clients in a readable format
  * Input: None
  * Output: None
*/
void printAllConnectedClients() {
  printf("\n*** PRINTING ALL CONNECTED CLIENTS ***\n");
  int i;
  unsigned long udpAddress;
  unsigned short udpPort;
  unsigned long tcpAddress;
  unsigned short tcpPort;
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
    udpAddress = ntohl(connectedClients[i].socketUdpAddress.sin_addr.s_addr);
    udpPort = ntohs(connectedClients[i].socketUdpAddress.sin_port);
    tcpAddress = ntohl(connectedClients[i].socketTcpAddress.sin_addr.s_addr);
    tcpPort = ntohs(connectedClients[i].socketTcpAddress.sin_port);
    if (udpAddress == 0 && udpPort == 0 && tcpAddress == 0 && tcpPort == 0) {
      continue;
    }
    printf("CONNECTED CLIENT %d\n", i);
    printf("UDP ADDRESS: %ld\n", udpAddress);
    printf("UDP PORT: %d\n", udpPort);
    printf("TCP ADDRESS: %ld\n", tcpAddress);
    printf("TCP PORT: %d\n\n", tcpPort);
  }
}

/*
  * Name: broadcastMessage
  * Purpose: Broadcast a plain text message to all clients except the sender
  * Input: 
  * - Udp socket to send the message out on
  * - Message to broadcast
  * - Who sent the message
  * Output: None
*/
void broadcastMessage(int udpSocketDescriptor, char* message, struct sockaddr_in* sender_addr) {
    // Send exactly the number of bytes that were received
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
        if (connectedClients[i].socketTcpAddress.sin_addr.s_addr != sender_addr->sin_addr.s_addr ||
            connectedClients[i].socketUdpAddress.sin_port != sender_addr->sin_port) {
            // Send only the number of bytes received (message_length)
            sendto(udpSocketDescriptor, message, strlen(message), 0, 
                   (struct sockaddr*)&connectedClients[i], sizeof(connectedClients[i]));
        }
    }
    if (debugFlag) {
      printf("Broadcast message: %s\n", message);
    }
}

/*
  * Name: sendCommandToChild
  * Purpose: Send a command to a child process through a pipe
  * Input: 
  * - The message to be sent
  * - The UDP address the command came from
  * Output:
  * - None
*/
void sendCommandToChild(char* message, struct sockaddr_in clientUDPAddress) {
  // Check all connected clients to find which one sent the command
  int i;
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {                                                     // Loop through all connected clients
    unsigned long currentConnectedClientUdpAddress;                                                 // Connected client UDP address
    unsigned short currentConnectedClientUdpPort;                                                   // Connected client port
    currentConnectedClientUdpAddress = ntohl(connectedClients[i].socketUdpAddress.sin_addr.s_addr); // Set the address
    currentConnectedClientUdpPort = ntohs(connectedClients[i].socketUdpAddress.sin_port);           // Set the port
    if (currentConnectedClientUdpAddress != ntohl(clientUDPAddress.sin_addr.s_addr)) {              // If the address doesn't match
      continue;                                                                                     // Keep looking
    }
    if (currentConnectedClientUdpPort != ntohs(clientUDPAddress.sin_port)) {                        // If address matches but the port doesn't
      continue;                                                                                     // Keep looking
    }
    // Address and port match
    // Send command to child process associated with the client that sent the message
    ssize_t bytesWrittenToChild;
    bytesWrittenToChild = write(connectedClients[i].serverParentToChildPipe[1], message, (strlen(message) + 1));  
  }
} 

