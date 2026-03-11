#include "clientUtils.h"
#include <debug.h>
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
    int     received = 0, currentPacketSize = 0;
    int     senderUsernameLen, senderMessageLen;
    char    readBuffer[MAX_READ_BUFFER_SIZE];

    while (1) {
        waitForSeverRespond(socketServer, 0);

        received += recv(socketServer, readBuffer + received, MAX_READ_BUFFER_SIZE - received, 0);

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

        // clearing buffer
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), received);
        SetEvent(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->event);
        ReleaseMutex(pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]->mutex);
    }
    _endthread();
}