#ifndef RADIKCHAT_SERVERUTILS_H
#define RADIKCHAT_SERVERUTILS_H

#include <stdint.h>
#include <ws2tcpip.h>

#define CLIENT_MAX_BUFFER_SIZE 1024
#define CLIENT_MAX_NICKNAME_SIZE 50

typedef struct sClientInfo
{
    socklen_t   addressSize;
    struct      sockaddr_storage address;
    SOCKET      socket;
    uint8_t     buffer[CLIENT_MAX_BUFFER_SIZE];
    size_t      receivedBytes;
    char        nickname[CLIENT_MAX_NICKNAME_SIZE + 1];
    bool        isLogined;

    struct sClientInfo* next;
} ClientInfo;

extern ClientInfo* clients;

void initServerUtils();

ClientInfo* getClient(SOCKET s);
void        deleteClient(ClientInfo* client);
char*       getClientAddress(ClientInfo *client);
fd_set      waitForClients(SOCKET server);
void        addClientToSet(ClientInfo *client);

#endif //RADIKCHAT_SERVERUTILS_H
