#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char* argv[]) {

	int status;
	struct addrinfo hints;
	struct addrinfo* serverAddressInfo;

	hints.ai_family = AF_INET;		// IPV4
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_protocol = 0;			// Any protocol
	hints.ai_flags = AI_PASSIVE;		// If node is null, will bind to IP of host
	
	getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);

	int socketDescriptor;
	socketDescriptor = socket(serverAddressInfo->ai_family, serverAddressInfo->ai_socktype, 0);

	bind(socketDescriptor, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen);

	listen(socketDescriptor, 10);		// Limit queued connections to 10

	sockaddr incomingAddress;
	int incomingSocketDescriptor;
	socklen_t sizeOfIncomingAddress = sizeof(incomingAddress);
	incomingSocketDescriptor = accept(socketDescriptor, &incomingAddress, &sizeOfIncomingAddress, 0);
	
	freeaddrinfo(serverAddressInfo);

	return 0;
}
