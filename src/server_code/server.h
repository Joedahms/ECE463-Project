#ifndef SERVER_H
#define SERVER_H

struct connectedClient {
  struct sockaddr_in socketUdpAddress;  // Address structure of connected client's UDP socket
  struct sockaddr_in socketTcpAddress;  // Address structure of connected client's TCP socket
  pid_t processId;                      // Process id of process forked to handle connection
  int serverParentToChildPipe[2];       // Pipe for parent to send data to child process
  int serverChildToParentPipe[2];       // Pipe for child to send data to parent process
};

void shutdownServer(int);               // Gracefully shutdown the server

void setupUdpSocket(struct sockaddr_in);
void setupTcpSocket(struct sockaddr_in);
int checkTcpSocket(struct sockaddr_in*, uint8_t);

void handleTcpConnection(struct sockaddr_in, uint8_t);
int findEmptyConnectedClient(uint8_t);

void printAllConnectedClients();

void broadcastMessage(int udpSocketDescriptor, char* message, struct sockaddr_in* sender_addr, int message_length);

#endif
