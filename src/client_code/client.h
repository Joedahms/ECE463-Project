#ifndef CLIENT_H
#define CLIENT_H

void shutdownClient(int);                                             // Gracefully shutdown client
void getUserInput(char*);                                             // Read user input from stdin
int checkForValidCommand(char*);                                      // Check if an entered command is valid
int putCommand(char*);                                                // Send a file to the server
int getCommand(char*);                                                // Request a file from the server
void sendTcpAddress(struct sockaddr_in, struct sockaddr_in, uint8_t); // Send client's TCP address to the server via UDP
void receiveMessageFromServer();                                      // Receive a message from the server

#endif
