#undef UNICODE
#include <stdio.h>
#include <client.h>
#include <winsock2.h> 
#include <ws2tcpip.h> 
#include <debug.h> 
#include <conio.h> 
#include <stdbool.h> 
#include <process.h> 
#include <windows.h>

#include <network_manager.h>

#define READ_BUFFER_SIZE 1024
#define ADDRESS_LEN 64
#define SERVICE_LEN 64

#define SERVER_ADDR "192.168.0.184"
#define SERVER_PORT "8899"
void socket_proc(void *arg);	// thread for socket select()
				//
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	WSADATA wsadata;

	DBG_DEBUG("Initializing winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		DBG_FATAL("Error: Failed to initialize winsock\n");
		return 1;
	}

	
	WSACleanup();
	return 0;
}

void socket_proc(void *arg)
{
	char *address = SERVER_ADDR, *port = SERVER_PORT;
	fd_set fd_reads;
	SOCKET socket_server;
	char *read_buffer;
	int bytes_received;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	if ((read_buffer = malloc(sizeof(*read_buffer) * READ_BUFFER_SIZE)) == NULL) {
		DBG_FATAL("Can't allocate space for read buffer\n");
		return;
	}

	socket_server = create_active_socket(address, port, SOCK_STREAM);
	if (socket_server == INVALID_SOCKET) {
		PRINT_WSA_ERROR(WSAGetLastError());
	} else {
		DBG_INFO("Successfully connected to server\n");
	}
	send(socket_server, "hello", 6, 0);

	while (1) {
		FD_ZERO(&fd_reads);
		FD_SET(socket_server, &fd_reads);

		if (select(0, &fd_reads, 0, 0, &timeout) < 0) {
			DBG_ERROR("select() failed. (%d)", WSAGetLastError());
			PRINT_WSA_ERROR(WSAGetLastError());
		}

		if (FD_ISSET(socket_server, &fd_reads)) {
			bytes_received = recv(socket_server, read_buffer, READ_BUFFER_SIZE, 0);
			if (bytes_received == 0) {
				printf("Server closed connection\n");
				return;
			} else if (bytes_received < 0) {
				DBG_ERROR("recv() failed. (%d)", WSAGetLastError());
				PRINT_WSA_ERROR(WSAGetLastError());
				return;
			} else {
				printf("%.*s", bytes_received, read_buffer);
			}
		}
	}

	if (closesocket(socket_server) == SOCKET_ERROR) {
		DBG_ERROR("closesocket() failed. Error code: (%d)\n", WSAGetLastError());
	}
}
