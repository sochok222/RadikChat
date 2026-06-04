#ifndef RADIKCHAT_SERVERUTILS_H
#define RADIKCHAT_SERVERUTILS_H

#include <stdint.h>
#include <ws2tcpip.h>

#define CLIENT_MAX_NICKNAME_SIZE 50

typedef struct sClientInfo
{
    socklen_t   addressSize;
    struct      sockaddr_storage address;
    SOCKET      socket;
    uint8_t     *buffer;
    size_t      bufferSize;
    HANDLE      mutex;
    int         receivedBytes;
    char        nickname[CLIENT_MAX_NICKNAME_SIZE + 1];
    bool        isLoggedIn;
    struct sClientInfo* next;
} ClientInfo;

extern ClientInfo* g_ciClients;

void initServerUtils();

ClientInfo* getClient(SOCKET s);
void        deleteClient(ClientInfo* client);
char*       getClientAddress(ClientInfo *client);
fd_set      waitForConnections(SOCKET server);
fd_set      waitForPackets(void);
void        addClientToSet(ClientInfo *client);

#endif //RADIKCHAT_SERVERUTILS_H
