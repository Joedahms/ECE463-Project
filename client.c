#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

int main(int argc, char* argv[]) {
	uint8_t debugFlag = 0;				// Can add conditional statements with this flag to print out extra info

	switch (argc) {					// Check how many command line arguments are passed
		case 1:
			printf("%s\n", "Running client in normal mode");
			break;
		case 2:
			if (strcmp(argv[1], "-d") == 0) {
				debugFlag = 1;
				printf("%s\n", "Running client in debug mode");
			}
			break;
		default:
	}
	
	int status;
	struct addrinfo hints;
	struct addrinfo* clientAddressInfo;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	getaddrinfo(NULL, "3940", &hints, &clientAddressInfo);

	int socketDescriptor;
	socketDescriptor = socket(clientAddressInfo->ai_family, clientAddressInfo->ai_socktype, 0);

	connect(socketDescriptor, clientAddressInfo->ai_addr, clientAddressInfo->ai_addrlen);

	char outgoingData[] = "TestingTestingTesting";
	send(socketDescriptor, outgoingData, strlen(outgoingData), 0);

	return 0;
}
