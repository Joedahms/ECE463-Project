#ifndef CLIENT_H
#define CLIENT_H

void shutdownClient(int);
void getUserInput(char*);
int checkForValidCommand(char*, const char**);
void sendUdpMessage(struct sockaddr_in, char*, uint8_t);
void putCommand();
void getCommand();

#endif
