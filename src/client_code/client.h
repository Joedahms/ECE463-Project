#ifndef CLIENT_H
#define CLIENT_H

void shutdownClient(int);
void getUserInput(char*);
int checkForValidCommand(char*);
int putCommand(char*);
int getCommand(char*);
void sendTcpAddress(struct sockaddr_in, struct sockaddr_in, uint8_t);
void receiveMessageFromServer();

#endif
