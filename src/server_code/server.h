#ifndef SERVER_H
#define SERVER_H

struct connectedClient {
  struct sockaddr_in clientAddress; // Address structure of connected client
  pid_t processId;                  // Process id of process forked to handle connection
  int serverParentToChildPipe[2];   // Pipe for parent to send data to child process
  int serverChildToParentPipe[2];   // Pipe for child to send data to parent process
};

void shutdownServer(int);                             // Gracefully shutdown the server
int handleErrorNonBlocking(int);                      // Handle error when "reading" from non blocking socket
void setupUdpSocket(struct sockaddr_in);
void setupTcpSocket(struct sockaddr_in);
int checkUdpSocket(char*, uint8_t);
int checkTcpSocket(int, struct sockaddr_in, uint8_t);
void handleTcpConnection(struct connectedClient[], size_t, struct sockaddr_in, uint8_t);

#endif
