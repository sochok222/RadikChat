#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include "contact.h"

#include <packetManager/packet.h>
#include <ws2tcpip.h>

extern SOCKET socketServer;

extern HANDLE socketServerMutex;
extern HANDLE socketThreadRunMutex;

extern HANDLE notificationsSemaphore;
extern HANDLE notificationThreadRunMutex;

typedef struct sSendMessageThreadArg
{
    SOCKET socket;
    Message *message;
    Contact *contact;
} SendMessageThreadArg;

void    initClientUtils();
fd_set  waitForSeverRespond(SOCKET server, struct timeval *timeout);
void    socketThread(void*);
void    sendMessageThread(void *args);

void printStatusErrorMessage(PacketStatus packetStatus);

#endif //RADIKCHAT_CLIENTUTILS_H
