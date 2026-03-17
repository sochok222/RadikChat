#include "clientUtils.h"
#include <debug.h>
#include <math.h>
#include <networkTypes.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fdMaster;

PendingRequest  *pendingRequests[MAX_PENDING_REQUESTS] = { 0 };
HANDLE          socketServerMutex;

void initClientUtils()
{
    FD_ZERO(&fdMaster);
}

fd_set  waitForSeverRespond(SOCKET server, struct timeval *timeout)
{
    fd_set fdReads;
    if (!FD_ISSET(server, &fdMaster))
        FD_SET(server, &fdMaster);

    fdReads = fdMaster;

    if (select(0, &fdReads, 0, 0, timeout) < 0) {
        DBG_FATAL("select() failed.\n");
        logWsaError(WSAGetLastError());
        exit(1); // TODO: add proper error handling
    }
    return fdReads;
}

void socketThread(void*)
{
    int     received = 0;
    char    readBuffer[MAX_READ_BUFFER_SIZE];

    while (WaitForSingleObject(socketThreadRunMutex, 75L) == WAIT_TIMEOUT) {
        waitForSeverRespond(socketServer, 0);

        received += recv(socketServer, readBuffer + received, MAX_READ_BUFFER_SIZE - received, 0);

        if (received == -1) {
            logWsaError(WSAGetLastError());
        }
        if (received == MAX_READ_BUFFER_SIZE) {
            // TODO handle this
        }

        if (received < 4)
            continue;

        if (*(int*)(readBuffer + PACKET_SIZE_OFFSET) > received)
            continue;

        if (pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)] == NULL) {
            DBG_FATAL("pendingRequests[] is NULL");
            break;
            // TODO handle this
        }

        WaitForSingleObject(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->mutex, INFINITE);
        // moving received packet to destination
        memcpy(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->buffer, readBuffer, *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->bufferSize = *(int*)(readBuffer + PACKET_SIZE_OFFSET);

        // clearing buffer
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        SetEvent(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->event);
        ReleaseMutex(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->mutex);
    }
    _endthread();
}

PendingRequest *createRequest(void)
{
    PendingRequest *request = NULL;

    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (pendingRequests[i] == NULL) {
            request = (pendingRequests[i] = malloc(sizeof(PendingRequest)));
            if (request == NULL) {
                DBG_FATAL("malloc failed");
                return NULL;
            }
            request->buffer = malloc(MAX_PENDING_REQUESTS_BUFFER_SIZE);
            if (request->buffer == NULL) {
                DBG_FATAL("malloc failed");
                free(request);
                return NULL;
            }
            request->requestId = i;
            request->bufferSize = 0;
            request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
            request->mutex = CreateMutex(NULL, FALSE, NULL);
            break;
        }
    }

    return request;
}

void deleteRequest(PendingRequest *request)
{
    free(request->buffer);
    free(request);
    request = NULL;
}