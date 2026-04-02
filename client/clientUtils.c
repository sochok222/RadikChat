#include "clientUtils.h"

#include "chatsManager.h"
#include "contactsManager.h"
#include "packetManager/packet.h"
#include "pendingOperation/delivery.h"
#include "pendingOperation/request.h"

#include <debug.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fdMaster;

HANDLE socketServerMutex;
HANDLE notificationsMutex;
HANDLE notificationThreadRunMutex;

static Packet   handleNewMessage(Packet p);
static void     addNotification(uint8_t *data);
static void     deleteNotification(Packet **notification);

void initClientUtils()
{
    notificationThreadRunMutex = CreateMutex(NULL, FALSE, NULL);
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
    PendingRequest *request = NULL;

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
            DBG_INFO("Received a delivery\n");
            addNotification(readBuffer);
            goto clear;
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
        clear:
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

static void addNotification(uint8_t *data)
{
    Packet *notification = malloc(sizeof(*notification));
    *notification = packetFromBytes(data);

    if (notification->data == NULL) {
        DBG_ERROR("notification data is NULL\n");
        if (notification->parseError == PARSE_ERROR_MALLOC_FAILED) {
            DBG_ERROR("Failed to allocate memory for notification\n");
        } else {
            DBG_ERROR("notification data size is not correct (%ull)\n", notification->size);
        }
        return;
    }

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        if (notifications[i] == NULL) {
            notifications[i] = notification;
            return;
        }
    }
    DBG_ERROR("Notifications are full\n");
    deleteNotification(&notification);
}

static void deleteNotification(Packet **notification)
{
    if (notification == NULL || *notification == NULL)
        return;
    if ((*notification)->data != NULL)
        free((*notification)->data);
    free(*notification);
    *notification = NULL;
}

void notificationThread(void*)
{
    Packet *notification;
    Packet respond;
    while (true) {
        WaitForSingleObject(notificationsMutex, INFINITE);
        for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
            if ((notification = notifications[i]) != NULL) {
                switch (notification->command) {
                case COMMAND_MESSAGE:
                    respond = handleNewMessage(*notification);
                    sendPacket(socketServer, respond, &socketServerMutex);
                    deletePacket(respond);
                    break;
                default:
                    break;
                }
                deleteNotification(&notifications[i]);
            }
        }
        ReleaseMutex(notificationsMutex);
        Sleep(1000);
    }
    _endthread();
}

static Packet handleNewMessage(Packet messagePacket)
{
    Packet respond = createPacket(TYPE_RESPOND, COMMAND_MESSAGE, STATUS_FAILURE, messagePacket.id);
    char *senderNickname, *receivedMessage;
    size_t readPos = 0;
    Contact *contact = contacts;

    if ((senderNickname = readPacketString(&messagePacket, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname from the message\n");
        respond.status = STATUS_CANT_READ;
        return respond;
    }

    while (contact != NULL) {
        if (strcmp(contact->nickname, senderNickname) != 0)
            break;
        contact = contact->next;
    }
    if (contact == NULL) {
        contact = createContact(senderNickname);
    }

    if ((receivedMessage = readPacketString(&messagePacket, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname from the message\n");
        respond.status = STATUS_CANT_READ;
        return respond;
    }

    addMessage(contact, receivedMessage, false);
    respond.status = STATUS_OK;

    return respond;
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