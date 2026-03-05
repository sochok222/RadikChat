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
	int         bytesReceived;
    int         packetType;
    int        respond;

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

	            if (bytesReceived <= 0) {
	                DBG_DEBUG("Disconnect from %s client", getClientAddress(client));
	                deleteClient(client);
	                break;
	            }
	            client->receivedBytes += bytesReceived;
	        }
            client = client->next;
	    }
	    // Process packets
	    client = clients;
	    while (client != NULL && client->receivedBytes > PACKET_HEADER_SIZE) {
	        packetType = *((int*)client->buffer);
	        if (client->receivedBytes > PACKET_HEADER_SIZE) {
	            if (packetType == TYPE_LOGIN) {
	                DBG_DEBUG("Received login packet from %s client\n", getClientAddress(client));
	                DBG_DEBUG("Expected to receive %d but received %d\n", *((int*)(client->buffer + PACKET_TYPE_SIZE)), client->receivedBytes - PACKET_HEADER_SIZE);
	                if (*((int*)(client->buffer + PACKET_TYPE_SIZE)) > (client->receivedBytes - PACKET_HEADER_SIZE)) { // Check if full message received
	                    client = client->next;
	                    continue;
	                }
	                if (!processLoginPacket(client)) {
	                    respond = TYPE_LOGIN_FAILURE;
	                } else
                        respond = TYPE_LOGIN_SUCCESS;
                    int sended = send(client->socket, (char*)&respond, sizeof(respond), 0);
	                DBG_INFO("Sended packet %d size\n", sended);
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