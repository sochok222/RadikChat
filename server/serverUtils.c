#include "serverUtils.h"
#include "debug.h"
#include "networkTypes.h"

#include <ws2tcpip.h>

#define MAX_CLIENT_ADDRESS_SIZE 30

ClientInfo *clients = NULL;
static fd_set fdMaster;


void initServerUtils()
{
    FD_ZERO(&fdMaster);
}

void addClientToSet(ClientInfo *client)
{
    FD_SET(client->socket, &fdMaster);
}

ClientInfo* getClient(SOCKET s)
{
    ClientInfo *it = clients;

    while (it != NULL) {
        if (it->socket == s)
            return it;
        it = it->next;
    }

    ClientInfo *newClient = calloc(1, sizeof(ClientInfo));
    newClient->isLogined = false;

    if (newClient == NULL) {
        DBG_FATAL("Out of memory\n");
        return NULL;
    }

    newClient->addressSize = sizeof(newClient->address);
    newClient->next = clients;
    clients = newClient;
    return newClient;
}

void deleteClient(ClientInfo *client)
{
    closesocket(client->socket);
    FD_CLR(client->socket, &fdMaster);
    ClientInfo **p = &clients;
    while(*p) {
        if (*p == client) {
            *p = client->next;
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

fd_set waitForClients(SOCKET server)
{
    struct timeval timeout;
    fd_set fdReads;
    if (!FD_ISSET(server, &fdMaster))
        FD_SET(server, &fdMaster);

    fdReads = fdMaster;

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if (select(0, &fdReads, 0, 0, &timeout) < 0) {
        DBG_FATAL("select() failed.\n");
        logWsaError(WSAGetLastError());
        exit(1); // TODO: add proper error handling
    }
    return fdReads;
}

bool processLoginPacket(ClientInfo *client)
{
    DBG_DEBUG("processLoginPacket\n");
    char    *clientName = client->buffer + PACKET_HEADER_SIZE;
    int     length = *((int*)(client->buffer + PACKET_TYPE_SIZE));
    int     packetSize = PACKET_HEADER_SIZE + length;

    if (length > CLIENT_MAX_NICKNAME_SIZE) {
        DBG_INFO("Client sent too long nickname\n")
        memcpy(client->buffer,
                client->buffer + packetSize,
                packetSize);
        client->receivedBytes -= packetSize;
        return false;
    }

    if (*(clientName + length - 1) != '\0')  {// Check if nickname is null-terminated
        DBG_INFO("Client sent non null-terminated packet\n")
        memcpy(client->buffer,
                client->buffer + packetSize,
                packetSize);
        client->receivedBytes -= packetSize;
        return false;
    }

    ClientInfo *iterator = clients;
    while (iterator != NULL) {
        if (strcmp(iterator->nickname, clientName) == 0) {
            DBG_INFO("Client sent nickname that is already registered %s\n", clientName);
            memcpy(client->buffer,
                    client->buffer + packetSize,
                    packetSize);
            client->receivedBytes -= packetSize;
            return false;
        }
        iterator = iterator->next;
    }
    strcpy(client->nickname, clientName);
    client->isLogined = true;

    // Clear buffer
    memcpy(client->buffer,
            client->buffer + packetSize,
            packetSize);
    client->receivedBytes -= packetSize;

    return true;
}

bool processMessagePacket(ClientInfo *client)
{
    return true;
}