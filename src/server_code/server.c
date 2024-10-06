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

// Max number of clients we will support
#define MAX_CLIENTS 100
#define MAX_FILENAME_SIZE 50
#define MAX_FILE_CONTENTS 1024

// Global variables
int socketDescriptor;
struct sockaddr_in serverAddress;

// Storage for uploaded files
char storedFiles[MAX_CLIENTS][MAX_FILENAME_SIZE];  // File names
char storedContents[MAX_CLIENTS][MAX_FILE_CONTENTS];  // File contents
int storedFileCount = 0;

// Client address storage
struct sockaddr_in client_addresses[MAX_CLIENTS];
socklen_t client_address_lengths[MAX_CLIENTS];
int client_count = 0;

// Forward declarations
void shutdownServer(int);
void handle_put_command(char* message, struct sockaddr_in* client_addr, socklen_t addr_len);
void handle_get_command(char* filename, struct sockaddr_in* client_addr, socklen_t addr_len);
void broadcast_message(char* message, struct sockaddr_in* sender_addr);
void add_client(struct sockaddr_in* client_addr, socklen_t addr_len);

// Main function
int main(int argc, char* argv[]) {
    signal(SIGINT, shutdownServer);

    // Set up server sockaddr_in data structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Set up socket
    printf("Setting up socket...\n");
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("Error setting up socket");
        exit(1);
    }

    // Bind socket
    printf("Binding socket...\n");
    int bindReturn = bind(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindReturn == -1) {
        perror("Error binding socket");
        exit(1);
    }

    char message[INITIAL_MESSAGE_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Continuously listen for new UDP packets
    while (1) {
        // Receive a message from a client
        int bytesReceived = recvfrom(socketDescriptor, message, INITIAL_MESSAGE_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytesReceived == -1) {
            perror("Error receiving message");
            exit(1);
        }
        message[bytesReceived] = '\0';  // Null-terminate received string

        // Add the client to the list if it's new
        add_client(&client_addr, client_addr_len);

        // Check if it's a command (starts with %)
        if (message[0] == '%') {
            if (strncmp(message, "%put ", 5) == 0) {
                handle_put_command(message, &client_addr, client_addr_len);
            } else if (strncmp(message, "%get ", 5) == 0) {
                char* filename = &message[5];
                handle_get_command(filename, &client_addr, client_addr_len);
            } else {
                printf("Unrecognized command: %s\n", message);
            }
        } else {
            // Plain text, broadcast the message to all clients
            broadcast_message(message, &client_addr);
        }
    }

    return 0;
}

// Store the uploaded file from a PUT command
void handle_put_command(char* message, struct sockaddr_in* client_addr, socklen_t addr_len) {
    char* filename = &message[5];
    if (storedFileCount >= MAX_CLIENTS) {
        printf("File storage full\n");
        return;
    }

    // Receive file contents from client
    char fileContents[MAX_FILE_CONTENTS];
    int bytesReceived = recvfrom(socketDescriptor, fileContents, MAX_FILE_CONTENTS, 0, NULL, NULL);
    if (bytesReceived > 0) {
        fileContents[bytesReceived] = '\0';  // Null-terminate the received string
        strncpy(storedFiles[storedFileCount], filename, MAX_FILENAME_SIZE);
        strncpy(storedContents[storedFileCount], fileContents, MAX_FILE_CONTENTS);
        storedFileCount++;
        printf("File '%s' stored from client\n", filename);
    } else {
        perror("Error receiving file contents");
    }
}

// Send the requested file to the client in response to a GET command
void handle_get_command(char* filename, struct sockaddr_in* client_addr, socklen_t addr_len) {
    for (int i = 0; i < storedFileCount; i++) {
        if (strcmp(storedFiles[i], filename) == 0) {
            sendto(socketDescriptor, storedContents[i], strlen(storedContents[i]), 0, (struct sockaddr*)client_addr, addr_len);
            printf("File '%s' sent to client\n", filename);
            return;
        }
    }
    printf("File '%s' not found\n", filename);
}

// Broadcast a plain text message to all clients except the sender
void broadcast_message(char* message, struct sockaddr_in* sender_addr) {
    for (int i = 0; i < client_count; i++) {
        if (client_addresses[i].sin_addr.s_addr != sender_addr->sin_addr.s_addr ||
            client_addresses[i].sin_port != sender_addr->sin_port) {
            sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr*)&client_addresses[i], client_address_lengths[i]);
        }
    }
    printf("Broadcast message: %s\n", message);
}

// Add a client to the list of connected clients
void add_client(struct sockaddr_in* client_addr, socklen_t addr_len) {
    for (int i = 0; i < client_count; i++) {
        if (client_addresses[i].sin_addr.s_addr == client_addr->sin_addr.s_addr &&
            client_addresses[i].sin_port == client_addr->sin_port) {
            return;  // Client is already in the list
        }
    }
    client_addresses[client_count] = *client_addr;
    client_address_lengths[client_count] = addr_len;
    client_count++;
    printf("Added new client\n");
}

// Shutdown the server gracefully
void shutdownServer(int signal) {
    close(socketDescriptor);
    printf("\nServer shutdown\n");
    exit(0);
}
