/*
  * Name:
  * Purpose:
  * Input: 
  * Output:
*/

#ifndef  NETWORK_NODE_H
#define NETWORK_NODE_H

#define PORT 3941
#define FILE_NAME_SIZE 50
#define MAX_FILE_SIZE 5000
#define INITIAL_MESSAGE_SIZE 100
#define MAX_CONNECTED_CLIENTS 100

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>

void checkCommandLineArguments(int, char**, uint8_t*); // Check how many command line arguments passed
int networkNodeConnect(const char*, int, struct sockaddr*, socklen_t);

char* packetAppend(char*, const char*, const char*);

int receiveBytes(int, char*, int, uint8_t);
int sendBytes(int, const char*, unsigned long int, uint8_t);
void sendUdpMessage(int, struct sockaddr_in, char*, uint8_t);

int checkStringForCommand(const char*);

void printReceivedMessage(struct sockaddr_in, int, char*, uint8_t);

int readFile(char*, char*, uint8_t);
int writeFile(char*, char*, size_t);

void fileNameFromCommand(char*, char*);

int checkUdpSocket(int, struct sockaddr_in*, char*, uint8_t);
int handleErrorNonBlocking(int);        // Handle error when "reading" from non blocking socket

#endif
