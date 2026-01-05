#include <stdio.h>
#include <client.h>
#include <winsock2.h> /* socket ... */
#include <ws2tcpip.h> /* getaddrinfo ... */
#include <debug.h>
#include <network_manager.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64


int main(void) 
{
	WSADATA wsadata;
	struct addrinfo hints, *server_address;
	char address[ADDRESS_LEN+1], service[SERVICE_LEN+1];
	SOCKET socket_server;

	DBG_DEBUG("Initializing winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		DBG_FATAL("Error: Failed to initialize winsock\n");
		return 1;
	}

	printf("Enter address: ");
	if (scanf("%s", address) != 1) {
		DBG_FATAL("Error: Failed to read address\n");
		WSACleanup();
		return 1;
	}
	printf("Enter service: ");
	if (scanf("%s", service) != 1) {
		DBG_FATAL("Error: Failed to read service\n");
		WSACleanup();
		return 1;
	}

	socket_server = create_active_socket(address, service, SOCK_STREAM);
	if (socket_server == INVALID_SOCKET) {
		PRINT_WSA_ERROR(WSAGetLastError());
	}
	DBG_INFO("sucessfully connected\n");

	if (closesocket(socket_server) == SOCKET_ERROR) {
		DBG_ERROR("Error: closesocket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}
