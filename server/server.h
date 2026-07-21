#ifndef RADIKCHAT_SERVER_H
#define RADIKCHAT_SERVER_H

#include "critical_section.h"
#include "tl_packet.h"
#include <mswsock.h>

#define MAX_BUFFER_SIZE ((uint32_t)(MAX_PACKET_SIZE * 4)) // 4 packets per io maximum

typedef enum IoOperation
{
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_ACCEPT,
} IoOperation;

typedef struct PerIoContext
{
    WSAOVERLAPPED   overlapped;
    char            buffer[MAX_BUFFER_SIZE];
    WSABUF          wsabuf;
    DWORD           total_bytes;
    DWORD           sent_bytes;
    IoOperation     io_operation;
    SOCKET          socket_accept;

    struct PerIoContext *io_context_forward;
} PerIoContext;

typedef struct PerSocketContext {
    SOCKET          socket;
    char            *nickname;
    // first io context in the list is the context for incoming io
    // all other io context are queue for outcoming data
    PerIoContext    *io_context;

    // These sockets are used only for IO_OP_ACCEPT
    // accept_socket -> socket which accepted
    // NOTE during other IO operations this field is not used
    SOCKET accept_socket;

    // Only one packet can be sent at the moment
    CriticalSection send_critical_section;
    bool            send_in_progress;

    struct PerSocketContext *ctxt_back;
    struct PerSocketContext *ctxt_forward;
} PerSocketContext;

extern PerSocketContext *g_clients;

DWORD WINAPI        worker_thread(LPVOID arg);
bool                send_to_client(PerSocketContext *per_socket_context, TlPacket *packet);
PerSocketContext    *update_completion_port(SOCKET s, IoOperation client_io, bool add_to_list);
void                close_client(PerSocketContext *per_socket_context, bool graceful);
PerSocketContext    *allocate_socket_context(SOCKET s, IoOperation client_io);
PerIoContext        *allocate_io_context();
void                add_io_context_to_socket_context(PerSocketContext *per_socket_context, PerIoContext *io_context);
void                delete_io_context_from_socket_context(PerSocketContext *per_socket_context, PerIoContext *io_context);
void                free_socket_context_list();
void                add_to_socket_context_list(PerSocketContext *per_socket_context);
void                delete_from_socket_context_list(PerSocketContext *per_socket_context);
void                delete_io_context(PerIoContext *per_io_context);

#endif //RADIKCHAT_SERVER_H