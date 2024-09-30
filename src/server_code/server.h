#ifndef SERVER_H
#define SERVER_H

void shutdownServer(int);                             // Gracefully shutdown the server
int handleErrorNonBlocking(int);                      // Handle error when "reading" from non blocking socket
void setupUdpSocket(struct sockaddr_in);
void setupTcpSocket(struct sockaddr_in);
int checkUdpSocket(char*, uint8_t);
void checkTcpSocket(struct sockaddr_in, uint8_t);

#endif
