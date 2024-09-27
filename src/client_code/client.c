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
int socketDescriptor;

struct sockaddr_in serverAddress;



// Forward declarations
void printFileInformation(const char*, struct stat);
void shutdownClient(int);

// Main
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownClient);

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

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
	
  /*
  // Setup server address
	serverAddress.sin_family = AF_INET;                              // IPV4
  serverAddress.sin_port = htons(3940);
*/

	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

  // Connect to the server
  //const char* nodeName = "server";
  //socketDescriptor = networkNodeConnect(nodeName, socketDescriptor, clientAddressInfo);

  // Constantly check user input for a put/get command
  while(1) {
    // Get user input and store in userInput buffer
    char* userInput = malloc(USER_INPUT_BUFFER_LENGTH);
    fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);
    userInput[strcspn(userInput, "\n")] = 0;                // Remove \n

    if (strlen(userInput) > 0) {  // User didn't just press return
      if (checkStringForCommand(userInput)) {  // User entered a command
        if (strncmp(userInput, "%put ", 5) == 0 || strncmp(userInput, "%get ", 5) == 0) { // Recognized command
          if (strlen(&userInput[5]) > 0) {                // User entered a file name following the command
            int fileAccess = access(&userInput[5], F_OK); // Check if the file exists
            if (fileAccess == -1) {                       // File does not exist
              char* errorMessage = malloc(1024);
              strcpy(errorMessage, strerror(errno));
              printf("Error: %s\n", errorMessage);
            } 
            else {
              if (debugFlag) {
                printf("Sending message to server:\n");
                printf("%s\n", userInput);
              }
              else {
                printf("Sending message to server...\n"); 
              }
              sendto(socketDescriptor, userInput, strlen(userInput), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
              printf("Message sent to server\n");
            }
          }
        }
        else {  // Unrecognized command
          printf("Please enter a valid command:\n");
          printf("%%put to send a file to the server\n");
          printf("%%get to request a file from the server\n");
        }
      }
      else { // User entered plain text to be sent to all other clients
        sendto(socketDescriptor, userInput, strlen(userInput), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
      }
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
  close(socketDescriptor);
  printf("\n");
  exit(0);
}
