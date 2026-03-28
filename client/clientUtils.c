#include "clientUtils.h"

#include "packetManager/packet.h"
#include "pendingOperation/request.h"

#include <debug.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fdMaster;

HANDLE socketServerMutex;
HANDLE notificationsMutex;
HANDLE notificationThreadRunMutex;

static void handleNewMessage(Packet p);

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

        if (*(int*)(readBuffer + PACKET_TYPE_OFFSET) == TYPE_DELIVERY) {

        }

        if ((request = pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]) == NULL) {
            DBG_ERROR("pendingRequests[id] is NULL");
            Sleep(1000);
            received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
            continue;
        }

        WaitForSingleObject(request->mutex, INFINITE);
        // moving received packet to destination
        writeToRequest(request, readBuffer, received);

        // clearing buffer
        // TODO handle too large PACKET_SIZE
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

void notifictionThread(void*)
{
    DBG_FUNC();
    Packet *notification;
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        WaitForSingleObject(notificationsMutex, INFINITE);
        if ((notification = notifications[i]) != NULL) {
            switch (notification->command) {
            case COMMAND_MESSAGE:
                handleNewMessage(*notification);
                break;
            default:
                break;
            }

            free(notification);
            notifications[i] = NULL;
        }
        ReleaseMutex(notificationsMutex);
    }
}

static void handleNewMessage(Packet packet)
{

}


void printStatusErrorMessage(PacketStatus packetStatus)
{
    switch (packetStatus) {
    case STATUS_FAILURE:
        DBG_ERROR("Status failed\n");
        break;
    case STATUS_CANT_READ:
        DBG_WARNING("Server can't read data from the packet\n");
        break;
    case STATUS_NOT_FOUND:
        DBG_WARNING("Packet not found\n");
        break;
    case STATUS_ALREADY_EXISTS:
        DBG_WARNING("Status already exists\n");
        break;
    case STATUS_SERVER_ERROR:
        DBG_WARNING("Server error\n");
        break;
    case STATUS_SIZE_TOO_BIG:
        DBG_WARNING("Packet size too big\n");
        break;
    case STATUS_ACTION_TO_HIMSELF:
        DBG_WARNING("Action to himself\n");
        break;
    default:
        DBG_ERROR("Unknown respond (%d)\n", packetStatus);
    }
}