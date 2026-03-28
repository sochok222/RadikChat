#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include <packetManager/packet.h>
#include <ws2tcpip.h>

extern SOCKET socketServer;

extern HANDLE socketServerMutex;
extern HANDLE socketThreadRunMutex;

extern HANDLE notificationsMutex;
extern HANDLE notificationThreadRunMutex;

void    initClientUtils();
fd_set  waitForSeverRespond(SOCKET server, struct timeval *timeout);
void    socketThread(void*);
void    notificationThread(void*);

void printStatusErrorMessage(PacketStatus packetStatus);

#endif //RADIKCHAT_CLIENTUTILS_H
