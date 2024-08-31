#ifndef  NETWORK_NODE_H
#define NETWORK_NODE_H

#include <stdint.h>

void receiveFile(int, uint8_t);
int receiveMessage(int, char*, int, uint8_t);

void sendFile(const char*, int, uint8_t);
void sendBytes(int, const char*, unsigned long int, uint8_t);

#endif
