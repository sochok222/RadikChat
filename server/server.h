#ifndef RADIKCHAT_SERVER_H
#define RADIKCHAT_SERVER_H

#include "tl_packet.h"

#include <mswsock.h>
#define MAX_BUFFER_SIZE 4096

typedef enum eIOOperation
{
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_ACCEPT,
} IO_Operation;

typedef struct sPerIOContext
{
    WSAOVERLAPPED   overlapped;
    char            buffer[MAX_BUFFER_SIZE];
    WSABUF          wsabuf;
    DWORD           total_bytes;
    DWORD           sent_bytes;
    IO_Operation    io_operation;
    SOCKET          socket_accept;
    TLPacket        *tl_packet;

    struct sPerIOContext *io_context_forward;
} PerIOContext;

typedef struct sPerSocketContext {
    SOCKET                      socket;
    char                        *nickname;
    PerIOContext                *io_context;
    CRITICAL_SECTION            io_critical_section;

    // These sockets are used only for IO_OP_ACCEPT
    // accept_socket -> socket which accepted
    // NOTE during other IO operations this field is not used
    SOCKET                      accept_socket;

    struct sPerSocketContext    *ctxt_back;
    struct sPerSocketContext    *ctxt_forward;
} PerSocketContext;

extern PerSocketContext *g_clients;

DWORD WINAPI        worker_thread(LPVOID arg);
PerSocketContext    *update_completion_port(SOCKET s, IO_Operation client_io, bool add_to_list);
void                close_client(PerSocketContext *per_socket_context, bool graceful);
PerSocketContext    *allocate_socket_context(SOCKET s, IO_Operation client_io);
PerIOContext        *allocate_io_context();
void                free_socket_context_list();
void                add_to_socket_context_list(PerSocketContext *per_socket_context);
void                delete_from_socket_context_list(PerSocketContext *per_socket_context);
void                delete_io_context(PerIOContext *per_io_context);

#endif //RADIKCHAT_SERVER_H