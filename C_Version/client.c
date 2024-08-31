#define USER_INPUT_BUFFER_LENGTH 40

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

#include "network_node.h"
#include "client.h"

// Global flags
uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

// Global variables (for signal handler)
int socketDescriptor;
struct addrinfo* clientAddressInfo;

//void sendFile(const char*, int);
//void sendBytes(int, const char*, unsigned long int, int);
void printFileInformation(const char*, struct stat);
void shutdownClient(int);

// Main
int main(int argc, char* argv[]) {
  signal(SIGINT, shutdownClient);

	switch (argc) {					// Check how many command line arguments are passed
		case 1:
			printf("Running client in normal mode\n");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) {	// Check if debug flag
				debugFlag = 1;
				printf("%s\n", "Running client in debug mode");
			}
			else {
				printf("Invalid usage of client");	// Could make this printout better
			}
			break;
    default:
			printf("Invalid usage of client");	// Could make this printout better
	}
	
	char* fileName = malloc(20);

	int status;
	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	getaddrinfo(NULL, "3940", &hints, &clientAddressInfo);

	socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0);

  // Constantly check user input for a put/get command
	while(1) {
    // Get user input and store in userInput buffer
		char userInput[USER_INPUT_BUFFER_LENGTH];
    fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);
    userInput[strcspn(userInput, "\n")] = 0;                // Remove \n

    // put
    // Send put along with file name
    // Send file
		if (userInput[0] == 'p' && userInput[1] == 'u' && userInput[2] == 't') {
      printf("Connecting to server...\n");
      int connectionStatus;
      connectionStatus = connect(socketDescriptor, clientAddressInfo->ai_addr, clientAddressInfo->ai_addrlen);
      // Check if connection was successful
      if (connectionStatus != 0) {
        char* errorMessage = malloc(1024);
        strcpy(errorMessage, strerror(errno));
        printf("Connection failed with error %s\n", errorMessage);
        exit(1);
      }
      printf("Connected to server...\n");
      sendFile(&userInput[4], socketDescriptor, debugFlag);  
      close(socketDescriptor);                                                                    // Close current connection
      socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0); // New socket descriptor for next connection
		}
    // get
    // Send get along with file name
    // Receive file
		else if (userInput[0] == 'g' && userInput[1] == 'e' && userInput[2] == 't') {
      
		}
    else {
      // Enter valid command (put/get)
    }
	}
	
	return 0;
}


/*
 * Name: sendFile
 * Purpose: Send a file along with its name to a waiting server
 * Input: 
 * - The name of the file to send
 * - Socket Descriptor of the socket to send the file out on
 * Output: None
 */
/*
void sendFile(const char* fileName, int socketDescriptor) {
	// Send the file name
  int fileNameLength = strlen(fileName);
  printf("Sending file: %s\n", fileName);
  if (debugFlag) {
    printf("Length of file name: %d\n", fileNameLength);
  }
	sendBytes(socketDescriptor, fileName, strlen(fileName), 0);

  // Open the file
	int fileDescriptor;
  printf("Opening file...\n");
	fileDescriptor = open(fileName, O_CREAT, O_RDWR);	// Create if does not exist + read and write mode
  printf("File Open\n");

  // Get the size of the file in bytes
	struct stat fileInformation;
	if (stat(fileName, &fileInformation) == -1) {
    printf("Stat Error\n");
    exit(1);
  };
	unsigned long int fileSize = fileInformation.st_size;

  // Read the contents of the file into the file buffer
	char* fileBuffer = malloc(100000);
  printf("Reading File...\n");
	read(fileDescriptor, fileBuffer, fileSize);
  printf("File Read\n");

	// Send the contents of the file
  printf("Sending file contents...\n");
	sendBytes(socketDescriptor, fileBuffer, fileSize, 0);
  printf("File contents sent\n");
}
*/

/*
 * Name: sendBytes
 * Purpose: Send a desired number of bytes out on a Socket
 * Input:
 * - Socket Descriptor of the socket to send the bytes with
 * - Buffer containing the bytes to send
 * - Amount of bytes to send
 * - Flags? (lowkey don't remember why I put this here)
 * Output: None
 * Notes: need to change variable names to be more ambiguous
 */
/*
void sendBytes(int socketDescriptor, const char* fileBuffer, unsigned long int fileSize, int flags) {
  if (debugFlag) {
    printf("File size: %ld\n", fileSize);
  }
  // Print the bytes to send
  printf("Bytes to be sent:\n\n");
	int i;
	for (i = 0; i < fileSize; i++) {
		printf("%c", fileBuffer[i]);
	}
  printf("\n \n");

	int bytesSent = 0;
	bytesSent = send(socketDescriptor, fileBuffer, fileSize, 0);
	if (bytesSent != -1) {						// No error
		if (debugFlag) {
			printf("Bytes sent: %d\n", bytesSent);
		}	
	}
	else {
		printf("Error: send failed\n");
	}
}
*/

/*
 * Name: printFileInformation
 * Purpose: Utilize the stat data structure to print out various bits of
 * info about a particular file. Currently only using it to print out the
 * size of the file.
 * Input: 
 * - The name of the file
 * - The stat data structure corrosponding to the file
 * Output: None
 */
void printFileInformation(const char* fileName, struct stat fileInformation) {
	printf("Information about %s:\n", fileName);
	printf("Total size, in bytes: %ld\n", fileInformation.st_size);			
}

void shutdownClient(int signal) {
  close(socketDescriptor);
	freeaddrinfo(clientAddressInfo);
  printf("\n");
  exit(0);
}
