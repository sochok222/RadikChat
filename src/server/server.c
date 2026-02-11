#include <stdio.h>
#include <winsock2.h>
#include <debug.h>
#include <socket_utils.h>
#include <ws2tcpip.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64

int main(void) 
{
	WSADATA wsadata;
	struct sockaddr_storage client_address;
	char address[ADDRESS_LEN+1], service[SERVICE_LEN+1];
	SOCKET socket_listen, socket_client;
	fd_set fd_master, fd_reads;
	char *read_buffer;
	int i, sockaddr_size;

	int bytes_received;
	read_buffer = malloc(sizeof(*read_buffer) * 1024);
	if (read_buffer == NULL)
                _tprintf(TEXT("Cant alloc for read buffer\n"));

        _tprintf(TEXT("Initializing winsock...\n"));
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
                fprintf(stderr, TEXT("Error: Failed to initialize winsock\n"));
		return 1;
	}

        _tprintf(TEXT("Enter service: "));
        if (scanf(TEXT("%s"), service) != 1) {
                fprintf(stderr, TEXT("Error: Failed to read service\n"));
		WSACleanup();
		return 1;
	}

	socket_listen = create_passive_socket(service, SOCK_STREAM, AF_INET, 10);

	FD_ZERO(&fd_master);
	FD_SET(socket_listen, &fd_master);
        _tprintf(TEXT("Waiting for connections...\n"));
	while (1) {
		fd_reads = fd_master;
		if (select(0, &fd_reads, 0, 0, 0) < 0) {
                        _ftprintf(stderr, TEXT("Error: select() failed. Error code (%d)\n"), WSAGetLastError());
			closesocket(socket_listen);
			WSACleanup();
			return 1;
		}
		for (i = 0; i < fd_reads.fd_count; i++) {
			if (fd_reads.fd_array[i] == socket_listen) {
				sockaddr_size = sizeof(client_address);
				socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &sockaddr_size);
				if (socket_client == INVALID_SOCKET)
                                        _tprintf(TEXT("INVALID\n"));
				FD_SET(socket_client, &fd_master);
                                _tprintf(TEXT("Client connected.\n"));
			} else {
				bytes_received = recv(fd_reads.fd_array[i], read_buffer, 1024, 0);
				if (bytes_received < 1) {
					FD_CLR(i, &fd_master);
					closesocket(i);
					return 0;
					continue;
				}

                                _tprintf(TEXT("%.*s"), bytes_received, read_buffer);
			}
		}
	}



	if (closesocket(socket_listen) == SOCKET_ERROR) {
                _ftprintf(stderr, TEXT("Error: closesocket() failed. Error code: (%d)\n"), WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}
