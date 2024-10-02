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
  struct sockaddr_in serverAddress;     // Socket address that clients should connect to
                                        // Same port is used for both UDP and TCP connections
  struct sockaddr_in clientUDPAddress;  // Client's UDP info
  struct sockaddr_in clientTCPAddress;  // Client's TCP info

  // Set up server sockaddr_in data structure
  memset(&serverAddress, 0, sizeof(serverAddress));   // 0 out
  serverAddress.sin_family = AF_INET;                 // IPV4
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  
  serverAddress.sin_port = htons(PORT);               // Port
  
  checkCommandLineArguments(argc, argv, &debugFlag);  // Check if user passed any arguments

  setupUdpSocket(serverAddress);  // Setup the UDP socket
  setupTcpSocket(serverAddress);  // Setup the TCP socket
  
  char* message = malloc(INITIAL_MESSAGE_SIZE); // Space for incoming messages
  
  // Whether or not data is available at the socket. If it is, what kind.
  int udpStatus;
  int tcpStatus;

  // Continously listen for new UDP packets and new TCP connections
  while (1) {
    udpStatus = checkUdpSocket(&clientUDPAddress, message, debugFlag);  // Check the UDP socket
    switch (udpStatus) {
      case 0:   // Nothing
        break;

      case 1:   // TCP/UDP relation info
                // Message contains TCP address and port
        message += 9;                           // Discard $address=
        char* tcpAddressString = malloc(20);    // Space for address string
        strncpy(tcpAddressString, message, 10); // Store the address
        message += 16;                          // Discard the address and $port=
        char* tcpPortString = malloc(20);       // Space for port string
        strncpy(tcpPortString, message, 5);     // Store the port

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
            connectedClients[i].socketUdpAddress.sin_addr.s_addr = clientUDPAddress.sin_addr.s_addr;  // Set UDP address
            connectedClients[i].socketUdpAddress.sin_port = clientUDPAddress.sin_port;                // Set UDP port
            break;                                                                                    // Don't loop through the rest of the connected clients
          }
        }
        if (debugFlag) {
          printAllConnectedClients(); 
        }
        break;  // Break case 1

      case 2:   // Plain text
                // Handle plain text here
        break;  // Break case 2

      case 3:   // Put command
                // Handle put command here
        break;  // Break case 3

      case 4:   // Get command
        // Check all connected clients to find which one sent the get command
        for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {     // Loop through all connected clients
          unsigned long currentConnectedClientUdpAddress; // Connected client address
          unsigned short currentConnectedClientUdpPort;   // Connected client port
          currentConnectedClientUdpAddress = ntohl(connectedClients[i].socketUdpAddress.sin_addr.s_addr); // Set the address
          currentConnectedClientUdpPort = ntohs(connectedClients[i].socketUdpAddress.sin_port);           // Set the port
          if (currentConnectedClientUdpAddress != ntohl(clientUDPAddress.sin_addr.s_addr)) {  // If the address doesn't match
            continue;                                                                         // Keep looking
          }
          if (currentConnectedClientUdpPort != ntohs(clientUDPAddress.sin_port)) {            // If address matches but the port doesn't
            continue;                                                                         // Keep looking
          }
          // Address and port match
          // Send command to child process associated with the client that sent the message
          write(connectedClients[i].serverParentToChildPipe[1], message, (strlen(message) + 1));  
        }
        break;  // Break case 4

      case 5:   // Invalid command
        break;  // Break case 5

      default:
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
  * Name: checkUdpSocket
  * Purpose: Check if there is an incoming message on a UDP port. If there is then
  * return an integer depending on the type of message
  * Input:
  * - Address of the UDP port that is receiving messages.
  * - Buffer to read message into
  * - Debug flag
  * Output: 
  * - 0: No incoming packets
  * - 1: Information about the UDP/TCP info relationship on the clien
  * - 2: Plain text message
  * - 3: Put command
  * - 4: Get command
  * - 5: Invalid command
*/
int checkUdpSocket(struct sockaddr_in* incomingAddress, char* message, uint8_t debugFlag) {
  // Check for incoming messages
  socklen_t incomingAddressLength = sizeof(incomingAddress);
  int bytesReceived = recvfrom(listeningUDPSocketDescriptor, message, INITIAL_MESSAGE_SIZE, 0, (struct sockaddr *)incomingAddress, &incomingAddressLength);
  int nonBlockingReturn = handleErrorNonBlocking(bytesReceived);

  if (nonBlockingReturn == 1) {                 // No incoming messages
    return 0;                                   // Return 0
  }
  
  // Print out UDP message
  printReceivedMessage(*incomingAddress, bytesReceived, message, debugFlag); 

  if (strncmp(message, "$address=", 9) == 0) {    // Received info about TCP/UDP relation
    printf("Received TCP/UDP relation info\n");
    return 1;                                     // Return 1
  }
  else if (checkStringForCommand(message) == 0) { // Message is plain text
    printf("Received plain text\n");
    return 2;                                     // return 2
  }
  else if (strncmp(message, "%put ", 5) == 0) {   // Received put command
    printf("Received put command\n"); 
    return 3;                                     // Return 3
  }
  else if (strncmp(message, "%get ", 5) == 0) {   // Received get command
    printf("Received get command\n");
    return 4;                                     // Return 4
  }
  else {                                          // Received invalid command
    printf("Received invalid command\n");
    return 5;                                     // Return 5
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
  * back and forth. Fork a new process. 
  * Input: 
  * - Socket address structure of the connecting client's TCP socket
  * - Debug flag
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
      char* fileContents = malloc(100000);
      if (strncmp(dataFromParent, "%get ", 5) == 0) {
        
          dataFromParent += 5;
          char* fileName = malloc(1000);
          strncpy(fileName, dataFromParent, strlen(dataFromParent));


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

          // Read out the contents of the file
          printf("Reading file...\n");
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
        }
      
        // Send the file contents
        printf("Sending packet...\n");
        int bytesSent = sendBytes(connectedTCPSocketDescriptor, fileContents, strlen(fileContents), debugFlag);
        printf("%d byte packet sent\n", bytesSent);
    }
    exit(0);
  }
  else {                                                    // Parent process
    if (debugFlag) {
      printf("Forked child PID: %d\n", processId);
    }
    connectedClients[availableConnectedClient].processId = processId;

    close(connectedClients[availableConnectedClient].serverParentToChildPipe[0]);  // Close read on parent -> child. Write on this pipe
    close(connectedClients[availableConnectedClient].serverChildToParentPipe[1]);  // Close write on child -> parent. Read on this pipe

    write(connectedClients[availableConnectedClient].serverParentToChildPipe[1], "hello", 6);
  }
}

int findEmptyConnectedClient(uint8_t debugFlag) {
  int i;
  for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
    int port = ntohs(connectedClients[i].socketTcpAddress.sin_port);
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
