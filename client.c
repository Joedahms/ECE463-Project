#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#include "client.h"

int main(int argc, char* argv[]) {
	
	int status;
	struct addrinfo hints;
	struct addrinfo* clientAddressInfo;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	getaddrinfo("some ip address or url?", "3940", &hints, &clientAddressInfo);

	return 0;
}
