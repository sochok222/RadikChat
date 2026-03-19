#include "serverUtils.h"

#include <debug.h>
#include <networkTypes.h>
#include <socketUtils.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64
#define PORT "1423"
#define BACKLOG 10

static int serverInit();

int main(void) 
{
	SOCKET      socketListen;
	fd_set      fdReads;
	int         bytesReceived, packetType;

    serverInit();

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
	        } else {
                DBG_DEBUG("New connection from %s\n", getClientAddress(client));
	            addClientToSet(client);
	        }
	    }

	    // Receive packets
	    ClientInfo *client = clients;
	    while (client != NULL) {
	        if (FD_ISSET(client->socket, &fdReads)) {
	            bytesReceived = recv(client->socket, client->buffer + client->receivedBytes, sizeof(client->buffer) - client->receivedBytes, 0);

	            if (bytesReceived <= 0 && errno != EAGAIN) {
	                DBG_DEBUG("Disconnect from %s client\n", getClientAddress(client));
	                deleteClient(client);
	                break;
	            }
	            client->receivedBytes += bytesReceived;
	        }
            client = client->next;
	    }
	    // Process packets
	    client = clients;
	    while (client != NULL) {
	        if (client->receivedBytes < PACKET_HEADER_SIZE || *(int*)(client->buffer + PACKET_SIZE_OFFSET) > client->receivedBytes) {
	            client = client->next;
	            continue;
	        }
	        packetType = *(int*)(client->buffer + PACKET_TYPE_OFFSET);
	        if (packetType == PACKET_LOGIN) {
	            processLoginPacket(client);
	        } else if (packetType == PACKET_CREATE_CHAT) {
	            processCreateChatPacket(client);
	        } else if (packetType == PACKET_MESSAGE) {
	            processMessagePacket(client);
	        }
	        // clear packet
	        client->receivedBytes -= *(int*)(client->buffer + PACKET_SIZE_OFFSET);
	        memmove(client->buffer, client->buffer + *(int*)(client->buffer + PACKET_SIZE_OFFSET), *(int*)(client->buffer + PACKET_SIZE_OFFSET));
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

static int serverInit()
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