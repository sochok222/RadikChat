#include "clientUtils.h"
#include <debug.h>
#include <networkTypes.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024
#define PENDING_REQUEST_BUFFER_SIZE 100
#define MAX_PENDING_REQUESTS_BUFFER_SIZE 1024

static fd_set fdMaster;

PendingRequest  *pendingRequests[MAX_PENDING_REQUESTS] = { 0 };
HANDLE          socketServerMutex;

void writeToRequest(PendingRequest *request, uint8_t *data, size_t size);

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
    uint8_t readBuffer[MAX_READ_BUFFER_SIZE];
    PendingRequest *request;

    while (WaitForSingleObject(socketThreadRunMutex, 75L) == WAIT_TIMEOUT) {
        waitForSeverRespond(socketServer, 0);

        received += recv(socketServer, (char*)readBuffer + received, MAX_READ_BUFFER_SIZE - received, 0);

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

        if ((request = pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]) == NULL) {
            DBG_FATAL("pendingRequests[] is NULL");
            break;
            // TODO handle this
        }

        WaitForSingleObject(request->mutex, INFINITE);
        // moving received packet to destination
        writeToRequest(request, readBuffer, received);

        // clearing buffer
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

PendingRequest *createRequest(void)
{
    PendingRequest *request = NULL;

    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (pendingRequests[i] == NULL) {
            pendingRequests[i] = malloc(sizeof(*request));
            request = pendingRequests[i];
            if (request == NULL) {
                DBG_FATAL("malloc failed");
                break;
            }
            request->data = malloc(PENDING_REQUEST_BUFFER_SIZE);
            if (request->data == NULL) {
                DBG_FATAL("malloc failed");
                free(request);
                break;
            }
            request->id = i;
            request->size = 0;
            request->capacity = PENDING_REQUEST_BUFFER_SIZE;
            request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
            request->mutex = CreateMutex(NULL, FALSE, NULL);
            break;
        }
    }

    return request;
}

void writeToRequest(PendingRequest *request, uint8_t *data, size_t size)
{
    if (request->capacity < request->size + size) {
        request->data = realloc(request->data, request->size + size);
    }
    memcpy(request->data + request->size, data, size);
}

void deleteRequest(PendingRequest *request)
{
    pendingRequests[request->id] = NULL;
    free(request->data);
    free(request);
    request = NULL;
}