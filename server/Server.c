#include "ServerUtils.h"

#include <Debug.h>
#include <NetworkTypes.h>
#include <SocketUtils.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64
#define PORT "1423"
#define BACKLOG 10

static int init();

int main(void) 
{
	SOCKET      socketListen;
	fd_set      fdReads;
	int         bytesReceived;
    int         packetType;
    char        respond;

    init();

    socketListen = createPassiveSocket(PORT, SOCK_STREAM, AF_INET, BACKLOG);

    DBG_INFO("Waiting for connections...\n");
	while (1) {
	    fdReads = waitForClients(socketListen);
	    if (FD_ISSET(socketListen, &fdReads)) {
	        ClientInfo *client = getClient(-1);

	        client->socket = accept(socketListen, (struct sockaddr *)&client->address, &client->addressSize);

	        if (client->socket == INVALID_SOCKET) {
	            DBG_ERROR("Failed to accept connection\n");
	            logWsaError(WSAGetLastError());
	        }

	        DBG_DEBUG("New connection from %s\n", getClientAddress(client));
	    }

	    // Receive packets
	    ClientInfo *client = clients;
	    while (client != NULL) {
	        if (FD_ISSET(client->socket, &fdReads)) {
	            bytesReceived = recv(client->socket, client->buffer + client->receivedBytes, sizeof(client->buffer) - client->receivedBytes, 0);

	            if (bytesReceived <= 0) {
	                DBG_DEBUG("Disconnect from %s client", getClientAddress(client));
	                deleteClient(client);
	                continue;
	            }
	            client->receivedBytes += bytesReceived;
	        }
            client = client->next;
	    }
	    // Process packets
	    client = clients;
	    while (client != NULL) {
	        packetType = *((int*)client->buffer);
	        if (client->receivedBytes > PACKET_HEADER_SIZE) {
	            if (packetType == TYPE_LOGIN) {
	                if (*((int*)(client->buffer + PACKET_TYPE_SIZE)) < (bytesReceived - PACKET_HEADER_SIZE)) // Check if full message received
	                    continue;
	                if (!processLoginPacket(client)) {
	                    respond = TYPE_LOGIN_FAILURE;
	                } else
                        respond = TYPE_LOGIN_SUCCESS;
                    send(client->socket, &respond, sizeof(respond), 0);
	            } else if (packetType == TYPE_MESSAGE) {
	                // Check if recipient nickname is full

	                // Check if message is full

	                // Process message
	            }
	        }
	        client = client->next;
	    }
	}



	if (closesocket(socketListen) == SOCKET_ERROR) {
	    DBG_ERROR("Can't close listen socket");
	    logWsaError(WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}

static int init()
{
	WSADATA wsadata;

    initDebug();
    initServerUtils();

    DBG_INFO("Initializing winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }
}