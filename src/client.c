#include <stdio.h>
#include <client.h>
#include <winsock2.h> /* socket ... */
#include <ws2tcpip.h> /* getaddrinfo ... */

#define ADDRESS_LEN 64
#define SERVICE_LEN 64

int main(void) 
{
	WSADATA wsadata;
	struct addrinfo hints, *server_address;
	char address[ADDRESS_LEN+1], service[SERVICE_LEN+1];
	SOCKET socket_server;

	printf("Initializing winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		fprintf(stderr, "Error: Failed to initialize winsock\n");
		return 1;
	}

	printf("Enter address: ");
	if (scanf("%s", address) != 1) {
		fprintf(stderr, "Error: Failed to read address\n");
		WSACleanup();
		return 1;
	}
	printf("Enter service: ");
	if (scanf("%s", service) != 1) {
		fprintf(stderr, "Error: Failed to read service\n");
		WSACleanup();
		return 1;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM; // Setting TCP connection type
	hints.ai_flags = AI_NUMERICHOST; // Address must be numeric string
	if (getaddrinfo(address, service, &hints, &server_address) != 0) {
		fprintf(stderr, "Error: getaddrinfo() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	if ((socket_server = socket(server_address->ai_family, server_address->ai_socktype, 
				    server_address->ai_protocol)) == INVALID_SOCKET) {
		fprintf(stderr, "Error: socket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	if (connect(socket_server, server_address->ai_addr, server_address->ai_addrlen) != 0) {
		fprintf(stderr, "Error: connect() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	freeaddrinfo(server_address);
	printf("Successfully connected to server\nExiting\n");

	if (closesocket(socket_server) == SOCKET_ERROR) {
		fprintf(stderr, "Error: closesocket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}
