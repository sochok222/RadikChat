#include <stdio.h>
#include <winsock2.h>
#include <debug.h>
#include <network_manager.h>
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

	socket_listen = create_passive_socket(service, SOCK_STREAM, AF_INET, 10);

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
			if (FD_ISSET(fd_reads.fd_array[i], &fd_reads)) {
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
