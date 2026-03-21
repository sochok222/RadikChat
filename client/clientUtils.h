#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include <ws2tcpip.h>
#include <winsock.h>
#include <stdint.h>

#define MAX_PENDING_REQUESTS 100


typedef struct sPendingRequest
{
    int id;

    HANDLE mutex;
    HANDLE event;

    size_t  size;
    size_t  capacity;
    uint8_t *data;
} PendingRequest;

void    initClientUtils();
fd_set  waitForSeverRespond(SOCKET server, struct timeval *timeout);
void    socketThread(void*);

PendingRequest  *createRequest(void);
void            deleteRequest(PendingRequest *request);

extern PendingRequest *pendingRequests[MAX_PENDING_REQUESTS];

extern SOCKET socketServer;
extern HANDLE socketServerMutex;
extern HANDLE socketThreadRunMutex;

#endif //RADIKCHAT_CLIENTUTILS_H
