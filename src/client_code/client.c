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
uint8_t debugFlag = 0;  // Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
int tcpSocketDescriptor;
int udpSocketDescriptor;

struct sockaddr_in serverAddress;

// Main
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownClient);

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  checkCommandLineArguments(argc, argv, &debugFlag);
 
	udpSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  tcpSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

  const char* nodeName = "server";
  printf("Making TCP connection to %s...\n", nodeName);
  tcpSocketDescriptor = networkNodeConnect(nodeName, tcpSocketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  struct sockaddr_in tcpAddress;
  socklen_t tcpAddressLength = sizeof(tcpAddress);
  getsockname(tcpSocketDescriptor, (struct sockaddr *)&tcpAddress, &tcpAddressLength);
  printf("tcp port: %d\n", ntohs(tcpAddress.sin_port));
  
  // Send $address=tcpAddress.sin_addr.s_addr$port=tcpAddress.sin_port
  char* tcpAddressMessage = malloc(1000);
  strcpy(tcpAddressMessage, "$address=");
  char* address = malloc(60);
  snprintf(address, 60, "%d", ntohl(tcpAddress.sin_addr.s_addr));
  strncat(tcpAddressMessage, address, strlen(address));
  printf("tcpAddressMessage: %s\n", tcpAddressMessage);
  strcat(tcpAddressMessage, "$port=");
  char* port = malloc(60);
  snprintf(port, 60, "%d", ntohs(tcpAddress.sin_port));
  strncat(tcpAddressMessage, port, strlen(port));
  printf("tcpAddressMessage: %s\n", tcpAddressMessage);
  sendUdpMessage(serverAddress, tcpAddressMessage, debugFlag);

  const char* validCommands[NUMBER_VALID_COMMANDS];
  validCommands[0] = "%put ";
  validCommands[1] = "%get ";

  // Constantly check user input
  while(1) {
    // Get user input and store in userInput buffer
    char* userInput = malloc(USER_INPUT_BUFFER_LENGTH);
    getUserInput(userInput);

    if (strlen(userInput) == 0) {  // User just pressed return
      continue;
    }

    sendUdpMessage(serverAddress, userInput, debugFlag);  // Send user input via UDP

    /*
    struct sockaddr_in udpAddress;
    socklen_t udpAddressLength = sizeof(udpAddress);
    getsockname(udpSocketDescriptor, (struct sockaddr *)&udpAddress, &udpAddressLength);

    printf("udp port: %d\n", ntohs(udpAddress.sin_port));
*/

    if (checkStringForCommand(userInput) == 0) {          // User entered plain text message
      continue;
    }

    if (checkForValidCommand(userInput, validCommands) == 0) { // Not a recognized command or an invalid file name
      printf("Please enter a valid command:\n");
      printf("%%put to send a file to the server\n");
      printf("%%get to request a file from the server\n");
      continue;
    }

    if (strncmp(userInput, "%put ", 5) == 0) { // Send file contents (put)
      putCommand();
    }
    else {                                     // Receive file contents (get)
      getCommand();
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
  close(udpSocketDescriptor);
  close(tcpSocketDescriptor);
  printf("\n");
  exit(0);
}

void getUserInput(char* userInput) {
  fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);
  userInput[strcspn(userInput, "\n")] = 0;                // Remove \n
}

int checkForValidCommand(char* userInput, const char** validCommands) {
  int i;
  for (i = 0; i < NUMBER_VALID_COMMANDS; i++) {         // Loop thru the valid commands
    if (strncmp(userInput, validCommands[i], 5) == 0) { // If the user entered a valid command
      if (strlen(&userInput[5]) > 0) {                  // User entered a file name following the command
        int fileAccess = access(&userInput[5], F_OK);   // Check if the file exists
        if (fileAccess == -1) {                         // File does not exist
          perror("File access error");
          return 0;
        }
        else {                                          // File does exist
          return 1;                                     // Return 1
        }
      }
    } 
  }                                                     // If the user didn't enter a valid command
  return 0;                                             // Return 0
}

void sendUdpMessage(struct sockaddr_in destinationAddress, char* message, uint8_t debugFlag) {
  if (debugFlag) {
    printf("Sending UDP message:\n");
    printf("%s\n", message);
  }
  else {
    printf("Sending UDP message...\n"); 
  }

  int sendtoReturnValue = 0;
  sendtoReturnValue = sendto(udpSocketDescriptor, message, strlen(message), 0, (struct sockaddr *)&destinationAddress, sizeof(destinationAddress));
  if (sendtoReturnValue == -1) {
    perror("UDP send error");
    exit(1);
  }
  else {
    printf("UDP message sent\n");
  }
}

void putCommand() {
  char* buffer = "Hello";
  sendBytes(tcpSocketDescriptor, buffer, strlen(buffer), debugFlag);
}

void getCommand() {
  //char* buffer2 = malloc(100);
  //receiveBytes(tcpSocketDescriptor, buffer2, strlen(buffer2), debugFlag);
}


