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
void    createRequest(PendingRequest request);
void    deleteRequest(PendingRequest request);
void    socketThread(void*);

extern PendingRequest *pendingRequests[MAX_PENDING_REQUESTS];
extern SOCKET socketServer;
extern HANDLE socketServerMutex;

#endif //RADIKCHAT_CLIENTUTILS_H
