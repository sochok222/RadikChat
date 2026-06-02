#include "serverUtils.h"
#include "../lib/utils/network/packetManager/packet.h"
#include "debug.h"

#include <ws2tcpip.h>

#define MAX_CLIENT_ADDRESS_SIZE 30
#define MAX_PENDING_DELIVERIES 100
#define MAX_CLIENT_BUFFER_SIZE 1024

ClientInfo *g_clients = NULL;
static fd_set fdClients;
static HANDLE fdClientsMutex;

void initServerUtils()
{
    fdClientsMutex = CreateMutex(NULL, FALSE, NULL);
    FD_ZERO(&fdClients);
}

void addClientToSet(ClientInfo *client)
{
    WaitForSingleObject(fdClientsMutex, INFINITE);
    FD_SET(client->socket, &fdClients);
    ReleaseMutex(fdClientsMutex);
}

ClientInfo* getClient(SOCKET s)
{
    DBG_FUNC();
    ClientInfo *it = g_clients;

    while (it != NULL) {
        if (it->socket == s)
            return it;
        it = it->next;
    }

    ClientInfo *newClient = calloc(1, sizeof(ClientInfo));

    newClient->buffer = malloc(sizeof(*newClient->buffer) * MAX_CLIENT_BUFFER_SIZE);
    newClient->bufferSize = MAX_CLIENT_BUFFER_SIZE;
    newClient->mutex = CreateMutex(NULL, FALSE, NULL);
    newClient->isLoggedIn = false;

    newClient->addressSize = sizeof(newClient->address);
    newClient->next = g_clients;
    g_clients = newClient;

    return newClient;
}

void deleteClient(ClientInfo *client)
{
    DBG_FUNC();
    closesocket(client->socket);
    WaitForSingleObject(fdClientsMutex, INFINITE);
    FD_CLR(client->socket, &fdClients);
    ReleaseMutex(fdClientsMutex);
    ClientInfo **p = &g_clients;
    while(*p) {
        if (*p == client) {
            *p = client->next;
            ReleaseMutex(client->mutex);
            free(client->buffer);
            free(client);
            return;
        }
        p = &(*p)->next;
    }
    DBG_ERROR("client not found\n");
}

char *getClientAddress(ClientInfo *client)
{
    static char clientAddress[MAX_CLIENT_ADDRESS_SIZE];

    getnameinfo((struct sockaddr*)&client->address,
                client->addressSize,
                clientAddress, sizeof(clientAddress), 0, 0,
                NI_NUMERICHOST);

    return clientAddress;
}

fd_set waitForConnections(SOCKET server)
{
    DBG_FUNC();
    static fd_set fdReads;
    FD_ZERO(&fdReads);
    FD_SET(server, &fdReads);

    if (select(0, &fdReads, 0, 0, NULL) < 0) {
        DBG_FATAL("waitForClients select() failed.\n");
        logWsaError(WSAGetLastError());
        exit(1);
    }

    return fdReads;
}

fd_set waitForPackets(void)
{
    DBG_FUNC();
    static fd_set fdResult;
    FD_ZERO(&fdResult);

    WaitForSingleObject(fdClientsMutex, INFINITE);
    fdResult = fdClients;
    ReleaseMutex(fdClientsMutex);

    if (select(0, &fdResult, 0, 0, 0) < 0) {
        DBG_FATAL("waitForPackets select() failed.\n");
        logWsaError(WSAGetLastError());
        exit(1);
    }

    return fdResult;
}
