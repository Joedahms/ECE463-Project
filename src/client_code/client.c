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
int checkUserInputForCommand(const char*);
void printFileInformation(const char*, struct stat);
void shutdownClient(int);
void receive_message_from_server();
void send_file(const char* filename);  // For sending files via %put
void receive_file(const char* filename);  // For receiving files via %get

// Main
int main(int argc, char* argv[]) {
    signal(SIGINT, shutdownClient);

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);  // Replace with server IP

    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

    // Constantly check user input for a put/get command
    fd_set read_fds;
    while (1) {
        // Use select to handle user input and server messages simultaneously
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);  // 0 is stdin (for user input)
        FD_SET(socketDescriptor, &read_fds);  // The socket for receiving server messages

        int activity = select(socketDescriptor + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
        }

        // If there's user input, send it to the server
        if (FD_ISSET(0, &read_fds)) {
            // Get user input and store in userInput buffer
            char userInput[USER_INPUT_BUFFER_LENGTH];
            fgets(userInput, USER_INPUT_BUFFER_LENGTH, stdin);
            userInput[strcspn(userInput, "\n")] = 0;  // Remove \n

            if (strlen(userInput) > 0) {  // User didn't just press return
                if (checkUserInputForCommand(userInput)) {  // User entered a command
                    if (strncmp(userInput, "%put ", 5) == 0) {  // Recognized %put command
                        const char* filename = &userInput[5];  // Extract filename
                        send_file(filename);  // Send the file to the server
                    } else if (strncmp(userInput, "%get ", 5) == 0) {  // Recognized %get command
                        const char* filename = &userInput[5];  // Extract filename
                        receive_file(filename);  // Request the file from the server
                    } else {  // Unrecognized command
                        printf("Please enter a valid command:\n");
                        printf("%%put to send a file to the server\n");
                        printf("%%get to request a file from the server\n");
                    }
                } else {  // User entered plain text to be sent to all other clients
                    sendto(socketDescriptor, userInput, strlen(userInput), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
                    printf("Plain text message sent to server\n");
                }
            }
        }

        // If the server has sent a message, receive it
        if (FD_ISSET(socketDescriptor, &read_fds)) {
            receive_message_from_server();
        }
    }

    return 0;
}

// Function to receive messages from the server
void receive_message_from_server() {
    char buffer[USER_INPUT_BUFFER_LENGTH];
    int bytesReceived = recvfrom(socketDescriptor, buffer, USER_INPUT_BUFFER_LENGTH, 0, NULL, NULL);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received string
        printf("Message from server: %s\n", buffer);
    } else {
        perror("Error receiving message from server");
    }
}

// Function to send a file to the server using %put
void send_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File open error");
        return;
    }

    printf("Sending file: %s\n", filename);

    char fileBuffer[USER_INPUT_BUFFER_LENGTH];
    size_t bytesRead;

    while ((bytesRead = fread(fileBuffer, 1, USER_INPUT_BUFFER_LENGTH, file)) > 0) {
        sendto(socketDescriptor, fileBuffer, bytesRead, 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    }

    fclose(file);
    printf("File sent to server: %s\n", filename);
}

// Function to request a file from the server using %get
void receive_file(const char* filename) {
    printf("Requesting file: %s\n", filename);
    
    // Send a message to request the file from the server
    char requestMessage[USER_INPUT_BUFFER_LENGTH];
    snprintf(requestMessage, sizeof(requestMessage), "GET %s", filename);
    sendto(socketDescriptor, requestMessage, strlen(requestMessage), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // Prepare to receive the file
    char fileBuffer[USER_INPUT_BUFFER_LENGTH];
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("File open error");
        return;
    }

    printf("Receiving file: %s\n", filename);

    int bytesReceived;
    while ((bytesReceived = recvfrom(socketDescriptor, fileBuffer, USER_INPUT_BUFFER_LENGTH, 0, NULL, NULL)) > 0) {
        fwrite(fileBuffer, 1, bytesReceived, file);
    }

    fclose(file);
    printf("File received: %s\n", filename);
}

/*
 * Name: checkUserInputForCommand
 * Purpose: Check if the user entered a command
 * Input: What the user entered
 * Output:
 * 1: User entered command
 * 0: User entered plain text
 */
int checkUserInputForCommand(const char* userInput) {
    if (userInput[0] == '%') {  // Check first character for '%'
        return 1;  // User entered command
    }
    return 0;  // User entered plain text
}

/*
 * Name: shutdownClient
 * Purpose: Gracefully shutdown the client.
 * Input: Signal received
 * Output: None
 */
void shutdownClient(int signal) {
    close(socketDescriptor);
    printf("\nClient shutdown\n");
    exit(0);
}
