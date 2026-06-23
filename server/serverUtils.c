#include "serverUtils.h"
#include "tlPacket.h"
#include "debug.h"

#include <ws2tcpip.h>

#define MAX_CLIENT_ADDRESS_SIZE 30
#define MAX_PENDING_DELIVERIES 100
#define MAX_CLIENT_BUFFER_SIZE 1024

ClientInfo *g_ci_clients = NULL;
static fd_set fd_clients;
static HANDLE fd_clients_mutex;

void init_server_utils()
{
    fd_clients_mutex = CreateMutex(NULL, FALSE, NULL);
    FD_ZERO(&fd_clients);
}

void add_client_to_set(ClientInfo *client)
{
    WaitForSingleObject(fd_clients_mutex, INFINITE);
    FD_SET(client->socket, &fd_clients);
    ReleaseMutex(fd_clients_mutex);
}

ClientInfo* get_client(SOCKET s)
{
    DBG_FUNC();
    ClientInfo *it = g_ci_clients;

    while (it != NULL) {
        if (it->socket == s)
            return it;
        it = it->next;
    }

    ClientInfo *new_client = calloc(1, sizeof(ClientInfo));

    new_client->buffer = malloc(sizeof(*new_client->buffer) * MAX_CLIENT_BUFFER_SIZE);
    new_client->buffer_size = MAX_CLIENT_BUFFER_SIZE;
    new_client->mutex = CreateMutex(NULL, FALSE, NULL);
    new_client->is_logged_in = false;

    new_client->address_size = sizeof(new_client->address);
    new_client->next = g_ci_clients;
    g_ci_clients = new_client;

    return new_client;
}

void delete_client(ClientInfo *client)
{
    DBG_FUNC();
    closesocket(client->socket);
    WaitForSingleObject(fd_clients_mutex, INFINITE);
    FD_CLR(client->socket, &fd_clients);
    ReleaseMutex(fd_clients_mutex);
    ClientInfo **p = &g_ci_clients;
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
    DBG_ERROR("client not found");
}

// FIXME make this function write to buffer passed as argument
char *get_client_address(ClientInfo *client)
{
    static char client_address[MAX_CLIENT_ADDRESS_SIZE];

    getnameinfo((struct sockaddr*)&client->address,
                client->address_size,
                client_address, sizeof(client_address), 0, 0,
                NI_NUMERICHOST);

    return client_address;
}

fd_set wait_for_connections(SOCKET server)
{
    DBG_FUNC();
    static fd_set fd_reads;
    FD_ZERO(&fd_reads);
    FD_SET(server, &fd_reads);

    if (select(0, &fd_reads, 0, 0, NULL) < 0) {
        DBG_FATAL("waitForClients select() failed.");
        log_wsa_error(WSAGetLastError());
        exit(1);
    }

    return fd_reads;
}

fd_set wait_for_packets(void)
{
    DBG_FUNC();
    static fd_set fd_result;
    FD_ZERO(&fd_result);

    WaitForSingleObject(fd_clients_mutex, INFINITE);
    fd_result = fd_clients;
    ReleaseMutex(fd_clients_mutex);

    if (select(0, &fd_result, 0, 0, 0) < 0) {
        DBG_FATAL("wait_for_packets select() failed.");
        log_wsa_error(WSAGetLastError());
        exit(1);
    }

    return fd_result;
}