#include "clientUtils.h"

#include "chatsManager.h"
#include "client.h"
#include "consoleOutput.h"
#include "contactsManager.h"
#include "packetManager/packet.h"
#include "pendingOperation/request.h"

#include <debug.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fdMaster;

HANDLE socketServerMutex;
HANDLE notificationsSemaphore;
HANDLE notificationThreadRunMutex;

static Packet   handleNewMessage(Packet p);

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
    notificationsSemaphore = CreateSemaphore(NULL, 0, 100, NULL);
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
            Packet notification, respond;
            DBG_INFO("Received a notification\n");
            notification = packetFromBytes(readBuffer);
            respond = handleNewMessage(notification);
            sendPacket(socketServer, respond, &socketServerMutex);
            deletePacket(notification);
            deletePacket(respond);

            goto clear;
        }

        if ((request = pendingRequests[*(int*)(readBuffer + PACKET_ID_OFFSET)]) == NULL) {
            DBG_ERROR("pendingRequests[id] is NULL");
            received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
            continue;
        }

        WaitForSingleObject(request->mutex, INFINITE);
        // move received packet to the destination
        writeToRequest(request, readBuffer, received);

        // clear buffer
        // TODO handle too large PACKET_SIZE
        clear:
        received -= *(int*)(readBuffer + PACKET_SIZE_OFFSET);
        memcpy(readBuffer, readBuffer + *(int*)(readBuffer + PACKET_SIZE_OFFSET), *(int*)(readBuffer + PACKET_SIZE_OFFSET));
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

void sendMessageThread(void *args)
{
    DBG_FUNC();
    Message         *message = ((SendMessageThreadArg*)args)->message;
    Contact         *contact = ((SendMessageThreadArg*)args)->contact;
    SOCKET          serverSocket = ((SendMessageThreadArg*)args)->socket;
    PendingRequest  *request = NULL;
    int             respond;
    Packet          pIn = {0}, pOut = {0};

    if (contact == NULL) {
        DBG_ERROR("Contact is null\n");
        free(args);
        return;
    }
    if (message == NULL) {
        DBG_ERROR("Message is null\n")
        free(args);
        return;
    }

    deleteRequest(&request);
    deletePacket(pIn); deletePacket(pOut);
    request = createRequest();
    if (request == NULL) {
        DBG_FATAL("Failed to create request\n");
        printNotification(formatError, "Can't send message");
        free(args);
        return;
    }
    pOut = createPacket(TYPE_REQUEST, COMMAND_MESSAGE, 0, request->id);
    addPacketString(&pOut, contact->nickname);
    addPacketString(&pOut, message->text);
    sendPacket(serverSocket, pOut, &socketServerMutex);

    WaitForSingleObject(request->event, INFINITE);
    WaitForSingleObject(request->mutex, INFINITE);

    pIn = packetFromBytes(request->data);
    deleteRequest(&request);
    if (pIn.data == NULL) {
        DBG_ERROR("Can't create packet from respond\n");
        free(args);
        return;
    }

    respond = pIn.status;

    switch (respond) {
    case STATUS_OK:
        DBG_INFO("Message sent\n");
        message->state = MESSAGE_SEND_SUCCESS;
        break;
    case STATUS_FAILURE:
        DBG_ERROR("Message sending failed\n");
        printNotification(formatError, "Can't send message");
        message->state = MESSAGE_SEND_FAILED;
        break;
    case STATUS_NOT_FOUND:
        DBG_ERROR("Server did not find contact\n");
        message->state = MESSAGE_SEND_FAILED;
        printNotification(formatError, "Contact is not logined");
        break;
    default:
        printStatusErrorMessage(respond);
        message->state = MESSAGE_SEND_FAILED;
    }

    SetEvent(appData.messageEvent);
    deleteRequest(&request);
    deletePacket(pIn); deletePacket(pOut);
    free(args);
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
        if (strcmp(contact->nickname, senderNickname) == 0)
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

    addMessage(contact, receivedMessage, false, MESSAGE_SEND_SUCCESS);
    respond.status = STATUS_OK;

    return respond;
}

void printStatusErrorMessage(PacketStatus packetStatus)
{
    switch (packetStatus) {
    case STATUS_FAILURE:
        DBG_ERROR("Status failed\n");
        printError("Status failed\n");
        break;
    case STATUS_CANT_READ:
        DBG_WARNING("Server can't read data from the packet\n");
        printError("Server can't read data from the packet\n");
        break;
    case STATUS_NOT_FOUND:
        DBG_WARNING("Not found\n");
        printError("Not found\n");
        break;
    case STATUS_ALREADY_EXISTS:
        DBG_WARNING("Status already exists\n");
        printError("Status already exists\n");
        break;
    case STATUS_SERVER_ERROR:
        DBG_WARNING("Server error\n");
        printError("Server error\n");
        break;
    case STATUS_SIZE_TOO_BIG:
        DBG_WARNING("Packet size too big\n");
        printError("Packet size too big\n");
        break;
    case STATUS_ACTION_TO_HIMSELF:
        DBG_WARNING("Action to himself\n");
        printError("Action to himself\n");
        break;
    default:
        DBG_ERROR("Unknown respond (%d)\n", packetStatus);
        printError("Unknown respond (%d)\n", packetStatus);
    }
}