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
uint8_t debugFlag = 0;  // Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
int tcpSocketDescriptor;
int udpSocketDescriptor;


// Main
int main(int argc, char* argv[]) {
  // Assign callback function for ctrl-c
  signal(SIGINT, shutdownClient);
  
  // Socket address data structure of the server
  struct sockaddr_in serverAddress;

  // Setup address of server to send to
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Check command line arguments
  checkCommandLineArguments(argc, argv, &debugFlag);
 
  // Setup sockets
	udpSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  tcpSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

  // Connect to the server
  const char* nodeName = "server";
  printf("Making TCP connection to %s...\n", nodeName);
  tcpSocketDescriptor = networkNodeConnect(nodeName, tcpSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  // Get the socket address structure for the local TCP connection to the server
  struct sockaddr_in tcpAddress;
  socklen_t tcpAddressLength = sizeof(tcpAddress);
  getsockname(tcpSocketDescriptor, (struct sockaddr *)&tcpAddress, &tcpAddressLength);
  printf("tcp port: %d\n", ntohs(tcpAddress.sin_port));

  // Send the local TCP connection info to the server
  sendTcpAddress(serverAddress, tcpAddress, debugFlag);

  fd_set read_fds;

  char* message = malloc(1000);
  struct sockaddr_in address;

  // Constantly check user input
  while(1) {

    // Use select to handle user input and server messages simultaneously
    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);  // 0 is stdin (for user input)
    FD_SET(udpSocketDescriptor, &read_fds);  // The socket for receiving server messages

    int activity = select(udpSocketDescriptor + 1, &read_fds, NULL, NULL, NULL);

    if (activity < 0 && errno != EINTR) {
        perror("select error");
    }

    if (FD_ISSET(0, &read_fds)) {
      // Get user input and store in userInput buffer
      char* userInput = malloc(USER_INPUT_BUFFER_LENGTH);
      getUserInput(userInput);

      // User just pressed return
      if (strlen(userInput) == 0) {  
        continue;
      }

      if (checkStringForCommand(userInput) == 0) {            // User entered plain text message
        sendUdpMessage(udpSocketDescriptor, serverAddress, userInput, debugFlag);  // Send user input via UDP
        //sendto(udpSocketDescriptor, userInput, strlen(userInput), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        printf("Plain text message sent to server\n");
        continue;
      }

      if (checkForValidCommand(userInput) == 0) {             // Not a recognized command or an invalid file name
        printf("Please enter a valid command:\n");
        printf("%%put to send a file to the server\n");
        printf("%%get to request a file from the server\n");
        continue;
      }

      char* fileName = malloc(FILE_NAME_SIZE);
      fileNameFromCommand(userInput, fileName);               // Extract the file name from the user input
      
      if (strncmp(userInput, "%put ", 5) == 0) {              // Put command
        sendUdpMessage(udpSocketDescriptor, serverAddress, userInput, debugFlag);  // Send user input(command) via UDP
        putCommand(fileName);                                 // Send the file
      }
      else {                                                  // Get command
        sendUdpMessage(udpSocketDescriptor, serverAddress, userInput, debugFlag);  // Send user input(command) via UDP
        if (getCommand(fileName) == -1) {                     // Receive the file
          printf("Failed to get file\n");
        }
      }
    }
    if (FD_ISSET(udpSocketDescriptor, &read_fds)) {
      receiveMessageFromServer();
    }
  }
	return 0;
} 

/*
 * Name: shutdownClient
 * Purpose: Gracefully shutdown the client.
 * Input: Signal received
 * Output: None
 */
void shutdownClient(int signal) {
  close(udpSocketDescriptor); // Close UDP socket
  close(tcpSocketDescriptor); // Close TCP socket
  printf("\n");
  exit(0);
}

/*
  * Name: getUserInput
  * Purpose: Get user input from standard in and remove the newline
  * Input: Buffer to store user input in
  * Output: None
*/
void getUserInput(char* userInput) {
  fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);  // Get the input
  userInput[strcspn(userInput, "\n")] = 0;            // Remove \n
}

/*
  * Name: checkForValidCommand
  * Purpose: Check if the user entered a valid command
  * Input: 
  * - User input
  * - Array of valid commands
  * Output:
  * - Whether or not the command is valid
*/
int checkForValidCommand(char* userInput) {
  int i;
  if (!(strlen(&userInput[5]) > 0)) {               // User didn't enter a file name after the command
      printf("Enter a file name after the command\n");
      return 0;  
  }

  if (strncmp(userInput, "%put ", 5) == 0) {        // User entered put
    int fileAccess = access(&userInput[5], F_OK);   // Check if the file exists
    if (fileAccess == -1) {                         // File does not exist
      perror("File access error");
    }
    return 1;                                       // Return 0
  }
  if (strncmp(userInput, "%get ", 5) == 0) {        // User entered get
    return 1;                                       // Return 1
  }
  return 0;                                         // Invalid command, return 0
}



/*
  * Name: putCommand
  * Purpose: Send a file to the server
  * Input: Name of the file to send
  * Output: None
*/
int putCommand(char* fileName) {
  char* fileContents = malloc(MAX_FILE_SIZE);
  int readFileReturn = readFile(fileName, fileContents, debugFlag);
  if (readFileReturn == -1) {
    printf("Put command error when reading file");
  }
  sendBytes(tcpSocketDescriptor, fileContents, strlen(fileContents), debugFlag);
}

/*
  * Name: getCommand
  * Purpose: Receive file from server and write it into local directory
  * Input: File name of requested file
  * Output: 
  * - -1: failure
  * - 0: Success
*/
int getCommand(char* fileName) {
  printf("Receiving file...\n");
  char* incomingFileContents = malloc(MAX_FILE_SIZE); // Space for file contents
  int numberOfBytesReceived;                          // How many bytes received
  numberOfBytesReceived = recv(tcpSocketDescriptor, incomingFileContents, MAX_FILE_SIZE, 0);

  int writeFileReturn = writeFile(fileName, incomingFileContents, numberOfBytesReceived);
  if (writeFileReturn == -1) {
    return -1;
  }
  
  if (debugFlag) {
    printf("Received file is %d bytes\n", numberOfBytesReceived);
    printf("Contents of received file:\n%s\n", incomingFileContents);
  }
  else {
    printf("Received file\n");
  }
  return 0;
}

/*
  * Name: sendTcpAddress
  * Purpose: Send the socket address structure of the tcp connection on the current process. Sent via UDP.
  * Input: 
  * - Where to send the address to
  * - The address to send
  * - Debug flag
  * Output: None
*/
void sendTcpAddress(struct sockaddr_in serverAddress, struct sockaddr_in tcpAddress, uint8_t debugFlag) {
  // Send $address=tcpAddress.sin_addr.s_addr$port=tcpAddress.sin_port
  char* tcpAddressMessage = malloc(1000);                         // Space for the message
  strcpy(tcpAddressMessage, "$address=");                         // Add $address=
  char* address = malloc(60);                                     // Space for the address string
  snprintf(address, 60, "%d", ntohl(tcpAddress.sin_addr.s_addr)); // Convert the address to a string
  strncat(tcpAddressMessage, address, strlen(address));           // Add the address to the message
  strcat(tcpAddressMessage, "$port=");                            // Add $port=
  char* port = malloc(60);                                        // Space for the port string
  snprintf(port, 60, "%d", ntohs(tcpAddress.sin_port));           // Convert the port to a string
  strncat(tcpAddressMessage, port, strlen(port));                 // Add the port to the message

  if (debugFlag) {
    printf("tcpAddressMessage: %s\n", tcpAddressMessage); 
  }

  sendUdpMessage(udpSocketDescriptor, serverAddress, tcpAddressMessage, debugFlag);    // Send the message to the server
}

/*
  * Name: receiveMessageFromServer
  * Purpose: Receive a UDP message from the server
  * Input: 
  * - None
  * Output:
  * - None
*/
void receiveMessageFromServer() {
    char buffer[USER_INPUT_BUFFER_LENGTH];
    memset(buffer, 0, sizeof(buffer));  // Clear the buffer before receiving a new message
    int bytesReceived = recvfrom(udpSocketDescriptor, buffer, USER_INPUT_BUFFER_LENGTH, 0, NULL, NULL);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received string
        printf("Message from server: %s\n", buffer);
    } else {
        perror("Error receiving message from server");
    }
}

