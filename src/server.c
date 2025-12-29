#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64

int main(void) 
{
	WSADATA wsadata;
	struct addrinfo hints, *bind_address = NULL;
	struct sockaddr_storage client_address;
	char address[ADDRESS_LEN+1], service[SERVICE_LEN+1];
	SOCKET socket_listen, socket_client;
	fd_set fd_master, fd_reads;
	int i, sockaddr_size;

	printf("Initializing winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		fprintf(stderr, "Error: Failed to initialize winsock\n");
		return 1;
	}

	printf("Enter service: ");
	if (scanf("%s", service) != 1) {
		fprintf(stderr, "Error: Failed to read service\n");
		WSACleanup();
		return 1;
	}

	printf("Configuring server address...\n");
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;	// Setting TCP connection type
	hints.ai_family = AF_INET; 		// socket will have ipv4 type
	hints.ai_flags = AI_PASSIVE;		// socket will be used for bind 
	if (getaddrinfo(NULL, service, &hints, &bind_address) != 0) {
		fprintf(stderr, "Error: getaddrinfo() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Creating socket...\n");
	if ((socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol)) == INVALID_SOCKET) {
		fprintf(stderr, "Error: socket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Binding socket...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
		fprintf(stderr, "Error: bind() failed. Error code: (%d)\n", WSAGetLastError());
		closesocket(socket_listen);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(bind_address);

	if (listen(socket_listen, 10) != 0) {
		fprintf(stderr, "Error: listen() failed. Error code: (%d)\n", WSAGetLastError());
		closesocket(socket_listen);
		WSACleanup();
		return 1;
	}

	FD_ZERO(&fd_master);
	FD_SET(socket_listen, &fd_master);
	printf("Waiting for connections...\n");
	while (1) {
		fd_reads = fd_master;
		if (select(0, &fd_reads, 0, 0, 0) < 0) {
			fprintf(stderr, "Error: select() failed. Error code (%d)\n", WSAGetLastError());
			closesocket(socket_listen);
			WSACleanup();
			return 1;
		}
		for (i = 0; i < fd_reads.fd_count; i++) {
			if (FD_ISSET(i, &fd_reads)) {
				sockaddr_size = sizeof(client_address);
				socket_client = accept(socket_listen, (struct sockaddr*)&socket_client, &sockaddr_size);
				FD_SET(socket_client, &fd_master);
				printf("Client connected.\n");
			}
		}
	}



	if (closesocket(socket_listen) == SOCKET_ERROR) {
		fprintf(stderr, "Error: closesocket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}
