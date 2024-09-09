#ifndef  NETWORK_NODE_H
#define NETWORK_NODE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>

typedef struct {
  char* delimiter;
  char* messageBegin;
  char* messageEnd;
  char* sendCommand;
}packetFields;

int networkNodeConnect(const char*, int, struct addrinfo*);

char* packetAppend(char*, const char*, const char*);

void receiveFile(int, packetFields, uint8_t);
int receiveBytes(int, char*, int, uint8_t);

void sendFile(const char*, int, packetFields, uint8_t);
int sendBytes(int, const char*, unsigned long int, uint8_t);

#endif
