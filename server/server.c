#include "packetProcessor.h"
#include "serverUtils.h"
#include <debug.h>
#include <packetManager/packet.h>
#include <process.h>
#include <socketUtils.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define ADDRESS_LEN 64
#define SERVICE_LEN 64
#define PORT "1423"
#define BACKLOG 10

CRITICAL_SECTION g_criticalSection;

static int serverInit();
static void acceptConnectionsThread(void *);
static void processClientsBuffersThread(void *);
static void receivePacketsThread(void *);

static DWORD WINAPI workingThread(LPVOID arg)
{
    HANDLE iocp = (HANDLE)arg;
    DWORD bytes;
    unsigned long long key;
    LPOVERLAPPED overlapped;
    while (true) {
        if (!GetQueuedCompletionStatus(iocp, &bytes, &key, &overlapped, INFINITE))
            break;
        if (!bytes) {
            // Client disconnected
        }
        switch (key) {
        case 1: // read
            break;
        case 0: // write
            break;
        default:
            break;
        }
    }
    return 0;
}

int main(void) 
{
	SOCKET      socketListen;
	fd_set      fdReads;
	int         bytesReceived, packetCommand;
    struct sockaddr_storage clientaddress;
    socklen_t clientaddressSize = sizeof(clientaddress);

    serverInit();

    socketListen = createPassiveSocket(PORT, SOCK_STREAM, AF_INET, BACKLOG);

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    DBG_INFO("Waiting for connections...\n");

    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    for (int i = 0; i < 3; i++) {
        HANDLE hWorking = CreateThread(NULL, 0, workingThread, iocp, 0, NULL);
        CloseHandle(hWorking);
    }

    InitializeCriticalSection(&g_criticalSection);

    while (1) {
        SOCKET clientSocket = WSAAccept(socketListen, NULL, NULL, NULL, 0);
    }

	// while (1) {
	//     fdReads = waitForConnections(socketListen);
	//     if (FD_ISSET(socketListen, &fdReads)) {
	//         ClientInfo *client = getClient(-1);
 //
	//         client->socket = accept(socketListen, (struct sockaddr *)&client->address, &client->addressSize);
 //
	//         if (client->socket == INVALID_SOCKET) {
	//             DBG_ERROR("Failed to accept connection\n");
	//             logWsaError(WSAGetLastError());
	//         } else {
 //                DBG_DEBUG("New connection from %s\n", getClientAddress(client));
	//             addClientToSet(client);
	//         }
	//     }
 //
	//     // Receive packets
	//     ClientInfo *client = g_clients;
	//     while (client != NULL) {
	//         if (FD_ISSET(client->socket, &fdReads)) {
	//             bytesReceived = recv(client->socket, client->buffer + client->receivedBytes, sizeof(client->buffer) - client->receivedBytes, 0);
 //
	//             if (bytesReceived <= 0 && errno != EAGAIN) {
	//                 DBG_DEBUG("Disconnect from %s client\n", getClientAddress(client));
	//                 deleteClient(client);
	//                 break;
	//             }
	//             client->receivedBytes += bytesReceived;
	//         }
 //            client = client->next;
	//     }
	//     // Process packets
	//     client = g_clients;
	//     while (client != NULL) { // TODO add maximum time to store packet that can't process
	//         if (client->receivedBytes < PACKET_HEADER_SIZE || *(size_t*)(client->buffer + PACKET_SIZE_OFFSET) > client->receivedBytes) {
	//             client = client->next;
	//             continue;
	//         }
	//         packetCommand = *(int*)(client->buffer + PACKET_COMMAND_OFFSET);
	//         if (packetCommand == COMMAND_LOGIN) {
	//             processLoginPacket(client);
	//         } else if (packetCommand == COMMAND_CREATE_CHAT) {
	//             processCreateChatPacket(client);
	//         } else if (packetCommand == COMMAND_MESSAGE) {
	//             processMessagePacket(client);
	//         }
	//         // clear packet
	//         client->receivedBytes -= *(int*)(client->buffer + PACKET_SIZE_OFFSET);
	//         memmove(client->buffer, client->buffer + *(int*)(client->buffer + PACKET_SIZE_OFFSET), client->receivedBytes);
	//         client = client->next;
	//     }
	// }

	if (closesocket(socketListen) == SOCKET_ERROR) {
	    DBG_ERROR("Can't close listen socket");
	    logWsaError(WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();

	return 0;
}

static void acceptConnectionsThread(void *arg)
{
    SOCKET socketListen = *(SOCKET*)arg;
    fd_set fdConnections;
    while (true) {
        FD_ZERO(&fdConnections);
        fdConnections = waitForConnections(socketListen);
        if (FD_ISSET(socketListen, &fdConnections)) {
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
    }
    _endthread();
}

static void processClientsBuffersThread(void *)
{
    ClientInfo *client;
    while (true) {
        client = g_clients;
        while (client != NULL) {

        }
    }
}

static void receivePacketsThread(void *)
{
    fd_set fdReceivePackets;
    int bytesReceived;

    while (true) {
        FD_ZERO(&fdReceivePackets);
        fdReceivePackets = waitForPackets();

        for (ClientInfo *client = g_clients; client != NULL; client = client->next) {
            if (FD_ISSET(client->socket, &fdReceivePackets)) {
                WaitForSingleObject(client->mutex, INFINITE);
                bytesReceived = recv(client->socket, (char*)client->buffer + client->receivedBytes, client->bufferSize - client->receivedBytes, 0);
                if (bytesReceived <= 0 && errno != EAGAIN) {
                    DBG_DEBUG("Disconnect from %s client\n", getClientAddress(client));
                    ReleaseMutex(client->mutex);
                    break;
                }
                client->receivedBytes += bytesReceived;
                ReleaseMutex(client->mutex);
            }
        }
    }
}

static int serverInit()
{
	WSADATA wsadata;

    initDebug(NULL);
    initServerUtils();

    DBG_INFO("Initializing winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        fprintf(stderr, "Error: Failed to initialize winsock\n");
        return 1;
    }
}