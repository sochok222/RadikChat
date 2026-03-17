#ifndef RADIKCHAT_CLIENTUTILS_H
#define RADIKCHAT_CLIENTUTILS_H

#include <ws2tcpip.h>
#include <winsock.h>

#define MAX_PENDING_REQUESTS 100
#define MAX_PENDING_REQUESTS_BUFFER_SIZE 1024

typedef struct sPendingRequest
{
    int requestId;

    HANDLE mutex;
    HANDLE event;

    int bufferSize;
    char *buffer;
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
