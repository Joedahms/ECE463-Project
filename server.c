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
	hints.ai_flags = AI_PASSIVE;		// Socket address suitable for binding a socket that will accept connections (if node is null)
	
	getaddrinfo(NULL, "3940", &hints, &serverAddressInfo);

		
	
	freeaddrinfo(serverAddressInfo);

	return 0;
}
