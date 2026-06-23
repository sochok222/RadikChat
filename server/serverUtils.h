#ifndef RADIKCHAT_SERVERUTILS_H
#define RADIKCHAT_SERVERUTILS_H

#include <stdint.h>
#include <ws2tcpip.h>

#define CLIENT_MAX_NICKNAME_SIZE 50

typedef struct sClientInfo
{
    socklen_t   address_size;
    struct      sockaddr_storage address;
    SOCKET      socket;
    uint8_t     *buffer;
    size_t      buffer_size;
    HANDLE      mutex;
    int         received_bytes;
    char        nickname[CLIENT_MAX_NICKNAME_SIZE + 1];
    bool        is_logged_in;
    struct sClientInfo* next;
} ClientInfo;

extern ClientInfo* g_ci_clients;

void init_server_utils();

ClientInfo* get_client(SOCKET s);
void        delete_client(ClientInfo* client);
char*       get_client_address(ClientInfo *client);
fd_set      wait_for_connections(SOCKET server);
fd_set      wait_for_packets(void);
void        add_client_to_set(ClientInfo *client);

#endif //RADIKCHAT_SERVERUTILS_H
