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
    DBG_FUNC();
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
    DBG_FUNC();
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

void processLoginPacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         respond = PACKET_LOGIN_SUCCESS, respondType = PACKET_LOGIN_RESPOND, respondSize;
    char        *nickname;

    // Check if nickname is null-terminated
    nickname = (client->buffer + PACKET_LOGIN_NICKNAME_OFFSET);
    if (*(nickname + *(int*)(client->buffer + PACKET_LOGIN_NICKNAME_SZ_OFFSET) - 1) != '\0') {
        DBG_ERROR("Received not null-terminated nickname\n");
        respond = PACKET_LOGIN_FAILURE;
    }

    // Search if client with same nickname is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found same nickname\n");
            respond = PACKET_LOGIN_FAILURE;
        }
        it = it->next;
    }

    // Save nickname to client
    strcpy(client->nickname, nickname);
    client->isLogined = true;

    respondSize = PACKET_HEADER_SIZE + sizeof(respond);

    // Send respond     size - type - id - respond
    send(client->socket, (char*)&respondSize, sizeof(respondSize), 0);
    send(client->socket, (char*)&respondType, sizeof(respondType), 0);
    send(client->socket, client->buffer + PACKET_ID_OFFSET, sizeof(int), 0);
    send(client->socket, (char*)&respond, sizeof(respond), 0);
}

void processCreateChatPacket(ClientInfo *client)
{

}

void processMessagePacket(ClientInfo *client)
{

}