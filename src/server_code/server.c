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
    memset(&(connectedClients[i].socketTcpAddress), 0, sizeof(connectedClients[i].socketTcpAddress));
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
  
  char* message = malloc(INITIAL_MESSAGE_SIZE);
  
  int udpStatus;
  int tcpStatus;
  // Continously listen for new UDP packets
  int j = 0;
  while (1) {
    udpStatus = checkUdpSocket(&clientUDPAddress, message, debugFlag);
    switch (udpStatus) {
      case 0:   // Nothing
        break;

      case 1:   // TCP/UDP relation info
        // Message contains TCP address and port
        //printf("%s\n", message);
        message += 9;
        char* tcpAddressString = malloc(20);
        strncpy(tcpAddressString, message, 10);
        //printf("%s\n", tcpAddressString);
        message += 16;
        char* tcpPortString = malloc(20);
        strncpy(tcpPortString, message, 5);
        //printf("%s\n", tcpPortString);

        long tcpAddressInteger = strtol(tcpAddressString, NULL, 10);
        long tcpPortInteger = strtol(tcpPortString, NULL, 10);

        int i; 
        for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
          unsigned long connectedClientAddress = ntohl(connectedClients[i].socketTcpAddress.sin_addr.s_addr);
          unsigned short connectedClientPort = ntohs(connectedClients[i].socketTcpAddress.sin_port);

          if (tcpAddressInteger == connectedClientAddress && tcpPortInteger == connectedClientPort) {
            connectedClients[i].socketUdpAddress.sin_addr.s_addr = clientUDPAddress.sin_addr.s_addr;
            connectedClients[i].socketUdpAddress.sin_port = clientUDPAddress.sin_port;
            break;
          }
        }

        printAllConnectedClients();
        break;

      case 2:   // Plain text
        break;

      case 3:   // Put command
        /*
        printf("UDP Address: %d\n", ntohl(clientUDPAddress.sin_addr.s_addr));
        printf("UDP Port: %d\n", ntohs(clientUDPAddress.sin_port));

        printf("TCP Address: %d\n", ntohl(connectedClients[0].socketTcpAddress.sin_addr.s_addr));
        printf("TCP Port: %d\n", ntohs(connectedClients[0].socketTcpAddress.sin_port));

        int incomingPort = ntohs(clientUDPAddress.sin_port);
        printf("Incoming get command from port: %d\n", incomingPort);
        */
        break;

      case 4:   // Get command
        for (i = 0; i < MAX_CONNECTED_CLIENTS; i++) {
          unsigned long currentConnectedClientUdpAddress;
          unsigned short currentConnectedClientUdpPort;
          currentConnectedClientUdpAddress = ntohl(connectedClients[i].socketUdpAddress.sin_addr.s_addr);
          currentConnectedClientUdpPort = ntohs(connectedClients[i].socketUdpAddress.sin_port);
          if (currentConnectedClientUdpAddress != ntohl(clientUDPAddress.sin_addr.s_addr)) {
            continue;
          }
          if (currentConnectedClientUdpPort != ntohs(clientUDPAddress.sin_port)) {
            continue;
          }
          write(connectedClients[i].serverParentToChildPipe[1], message, (strlen(message) + 1));
        }
        break;

      case 5:   // Invalid command
        break;

      default:
        
    }

    tcpStatus = checkTcpSocket(&clientTCPAddress, debugFlag);
    if (tcpStatus == 0) { // No data to be read
      ; 
    }
    else {                // Data to be read
      handleTcpConnection(clientTCPAddress, debugFlag);

      if (debugFlag) {
        printf("\ncc0 port: %d\n", ntohs(connectedClients[0].socketTcpAddress.sin_port));
        printf("cc0 address: %d\n", connectedClients[0].socketTcpAddress.sin_addr.s_addr);
        printf("cc0 pid: %d\n", connectedClients[0].processId);

        printf("cc1 port: %d\n", ntohs(connectedClients[1].socketTcpAddress.sin_port));
        printf("cc1 address: %d\n", connectedClients[1].socketTcpAddress.sin_addr.s_addr);
        printf("cc1 pid: %d\n\n", connectedClients[1].processId);
      }
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
// Check to see if any messages queued at UDP socket
int checkUdpSocket(struct sockaddr_in* incomingAddress, char* message, uint8_t debugFlag) {
  socklen_t incomingAddressLength = sizeof(incomingAddress);
  int bytesReceived = recvfrom(listeningUDPSocketDescriptor, message, INITIAL_MESSAGE_SIZE, 0, (struct sockaddr *)incomingAddress, &incomingAddressLength);
  int nonBlockingReturn = handleErrorNonBlocking(bytesReceived);

  if (nonBlockingReturn == 1) {                 // No incoming packets
    return 0;
  }
  
  // Print out UDP message
  printReceivedMessage(*incomingAddress, bytesReceived, message, debugFlag); 

  if (strncmp(message, "$address=", 9) == 0) {  // Received info about TCP/UDP relation
    printf("Received TCP/UDP relation info\n");
    return 1;
  }
  else if (checkStringForCommand(message) == 0) {    // Message is plain text
    return 2;                                   // return 1
  }
  else if (strncmp(message, "%put ", 5) == 0) { // Received put command
    printf("Received put command\n"); 
    return 3;                                   // Return 2
  }
  else if (strncmp(message, "%get ", 5) == 0) { // Received get command
    printf("Received get command\n");
    return 4;                                       // Return 3
  }
  else {                                            // Received invalid command
    printf("Received invalid command\n");
    return 5;                                       // Return 4
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
  connectedClients[availableConnectedClient].socketTcpAddress = clientTCPAddress;
  connectedClients[availableConnectedClient].serverParentToChildPipe;
  connectedClients[availableConnectedClient].serverChildToParentPipe;

  // Setup pipes
  pipe(connectedClients[availableConnectedClient].serverParentToChildPipe);
  pipe(connectedClients[availableConnectedClient].serverChildToParentPipe);

  // Fork a new process for the client
  pid_t processId;
  if ((processId = fork()) == -1) {                         // Fork error
    perror("Fork error"); 
  }
  else if (processId == 0) {                                // Child process
    close(connectedClients[availableConnectedClient].serverParentToChildPipe[1]);  // Close write on parent -> child.
    close(connectedClients[availableConnectedClient].serverChildToParentPipe[0]);  // Close read on child -> parent.

    //char dataFromParent[100];
    //char test[100];
    char* dataFromParent = malloc(100);
    char* test = malloc(100);
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
