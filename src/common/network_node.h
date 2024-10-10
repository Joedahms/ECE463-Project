// Template for function comments
/*
  * Name:
  * Purpose:
  * Input: 
  * Output:
*/

#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#define PORT 3941                                                       // Port server is listening on
#define FILE_NAME_SIZE 50                                               // Maximum file name length in bytes
#define MAX_FILE_SIZE 5000                                              // Maximum file length in bytes
#define INITIAL_MESSAGE_SIZE 100                                        // Max size of the initial calibration message
#define MAX_CONNECTED_CLIENTS 100                                       // Maximum number of clients that can be connected to the server
#define USER_INPUT_BUFFER_LENGTH 40                                     // Max size of message a user can input

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>

void checkCommandLineArguments(int, char**, uint8_t*);                  // Check how many command line arguments passed
int networkNodeConnect(const char*, int, struct sockaddr*, socklen_t);  // Connect to another network node

// Functions to send and receive messages
int sendBytes(int, const char*, unsigned long int, uint8_t);            // Send bytes with send()
int receiveBytes(int, char*, int, uint8_t);                             // Receive bytes with recv()
void sendUdpMessage(int, struct sockaddr_in, char*, uint8_t);           // Send a message over UDP

int checkStringForCommand(const char*);                                 // Checks a string for if it is a command

void printReceivedMessage(struct sockaddr_in, int, char*, uint8_t);     // Print out message along with its source

// File I/O
int readFile(char*, char*, uint8_t);                                    // Read from a file with read()
int writeFile(char*, char*, size_t);                                    // Write to a file with write()

void fileNameFromCommand(char*, char*);                                 // Pull out the filename from a command string

int checkUdpSocket(int, struct sockaddr_in*, char*, uint8_t);           // Check a UDP socket to see if it has any data in the queue
int handleErrorNonBlocking(int);                                        // Handle error when "reading" from non blocking socket

#endif
