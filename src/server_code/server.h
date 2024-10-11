#ifndef SERVER_H
#define SERVER_H

// Data about a client connected to the server
struct connectedClient {
  struct sockaddr_in socketUdpAddress;  // Address structure of connected client's UDP socket
  struct sockaddr_in socketTcpAddress;  // Address structure of connected client's TCP socket
  pid_t processId;                      // Process id of process forked to handle connection
  int serverParentToChildPipe[2];       // Pipe for parent to send data to child process
  int serverChildToParentPipe[2];       // Pipe for child to send data to parent process
};

void shutdownServer(int);                                 // Gracefully shutdown the server

void setupUdpSocket(struct sockaddr_in);                  // Setup a UDP socket
void setupTcpSocket(struct sockaddr_in);                  // Setup a TCP socket
int checkTcpSocket(struct sockaddr_in*, uint8_t);         // Check a TCP socket

void handleTcpConnection(struct sockaddr_in, uint8_t);    // Handle an incoming TCP connection
int findEmptyConnectedClient(uint8_t);                    // Find an empty connectedClient in an array of them

void printAllConnectedClients();                          // Print out all connected clients. Debug purposes

void broadcastMessage(int, char*, struct sockaddr_in *);  // Send a message to all connected clients except the one who send it
void sendCommandToChild(char*, struct sockaddr_in);       // Send a command to a child process through a pipe

#endif
