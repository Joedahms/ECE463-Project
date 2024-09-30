#ifndef  NETWORK_NODE_H
#define NETWORK_NODE_H

#define PORT 3941
#define FILE_NAME_SIZE 50
#define MAX_FILE_SIZE 5000
#define INITIAL_MESSAGE_SIZE 100
#define NUMBER_VALID_COMMANDS 2

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>

typedef struct {
  char* delimiter;
  char* messageBegin;
  char* messageEnd;
  char* putCommand;
  char* getCommand;
}packetFields;

void checkCommandLineArguments(int, char**, uint8_t*); // Check how many command line arguments passed
int networkNodeConnect(const char*, int, struct sockaddr*, socklen_t);

char* packetAppend(char*, const char*, const char*);

int receivePacket(int, char*, int, packetFields, uint8_t);
int receiveBytes(int, char*, int, uint8_t);

void sendPacket(const char*, int, packetFields, char* command, uint8_t);
int sendBytes(int, const char*, unsigned long int, uint8_t);

int checkStringForCommand(const char*);

#endif
