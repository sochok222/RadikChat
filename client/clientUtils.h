#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include "contact.h"

#include <tlPacket.h>
#include <ws2tcpip.h>

extern SOCKET socket_server;

extern HANDLE socket_server_mutex;
extern HANDLE socket_thread_run_mutex;

extern HANDLE notifications_semaphore;
extern HANDLE notification_thread_run_mutex;

typedef struct sSendMessageThreadArg
{
    SOCKET socket;
    Message *message;
    Contact *contact;
} SendMessageThreadArg;

void    init_client_utils();
fd_set  wait_for_sever_respond(SOCKET server, struct timeval *timeout);
void    socket_thread(void*);
void    send_message_thread(void *args);

void print_status_error_message(ServerRespond packet_status);

#endif //RADIKCHAT_CLIENTUTILS_H
