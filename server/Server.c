#include <Debug.h>
#include <SocketUtils.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <NetworkTypes.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64
#define PORT "1423"

int main(void) 
{
	WSADATA wsadata;
	struct sockaddr_storage clientAddress;
	SOCKET socketListen, socketClient;
	fd_set fdMaster, fdReads;
	char *readBuffer;
	int i, bytesReceived;
    socklen_t sockaddrSize;

    DBG_INFO("Initializing winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
                fprintf(stderr, "Error: Failed to initialize winsock\n");
		return 1;
	}

	socketListen = createPassiveSocket(service, SOCK_STREAM, AF_INET, 10);

	FD_ZERO(&fdMaster);
	FD_SET(socketListen, &fdMaster);
    printf("Waiting for connections...\n");
	while (1) {
		fdReads = fdMaster;
		if (select(0, &fdReads, 0, 0, 0) < 0) {
		    fprintf(stderr, "Error: select() failed. Error code (%d)\n", WSAGetLastError());
			closesocket(socketListen);
			WSACleanup();
			return 1;
		}
		for (i = 0; i < fdReads.fd_count; i++) {
			if (fdReads.fd_array[i] == socketListen) {
				sockaddrSize = sizeof(clientAddress);
				socketClient = accept(socketListen, (struct sockaddr*)&clientAddress, &sockaddrSize);
				if (socketClient == INVALID_SOCKET)
				    printf("INVALID\n");
				FD_SET(socketClient, &fdMaster);
			    printf("Client connected.\n");
			} else {
				bytesReceived = recv(fdReads.fd_array[i], readBuffer, 1024, 0);

				if (bytesReceived < 1) {
				    FD_CLR(i, &fdMaster);
				    closesocket(i);
				    return 0;
				    continue;
				}
			}
		}
	}



	if (closesocket(socketListen) == SOCKET_ERROR) {
                _ftprintf(stderr, "Error: closesocket() failed. Error code: (%d)\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}
