#include "clientUtils.h"

#include "chatsManager.h"
#include "client.h"
#include "consoleOutput.h"
#include "contactsManager.h"
#include "tlPacket.h"
#include "request.h"

#include <debug.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fdClients;

HANDLE socketServerMutex;
HANDLE notificationsSemaphore;
HANDLE notificationThreadRunMutex;

static TLPacket   handleNewMessage(TLPacket p);

void initClientUtils()
{
    notificationThreadRunMutex = CreateMutex(NULL, FALSE, NULL);
    FD_ZERO(&fdClients);
}

fd_set  waitForSeverRespond(SOCKET server, struct timeval *timeout)
{
    fd_set fdReads;
    if (!FD_ISSET(server, &fdClients))
        FD_SET(server, &fdClients);

    fdReads = fdClients;

    if (select(0, &fdReads, 0, 0, timeout) < 0) {
        DBG_FATAL("select() failed.");
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
    Request *request = NULL;

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

        if (*(int*)(readBuffer + PKT_SIZE_OFFSET) > received)
            continue;

        // if (*(int*)(readBuffer + PACKET_TYPE_OFFSET) == TYPE_DELIVERY) {
        //     TLPacket notification, respond;
        //     DBG_INFO("Received a notification");
        //     notification = packetFromBytes(readBuffer);
        //     respond = handleNewMessage(notification);
        //     sendPacket(socketServer, respond, &socketServerMutex);
        //     deletePacket(notification);
        //     deletePacket(respond);
        //
        //     int packetSize = *(int*)(readBuffer + PKT_SIZE_OFFSET);
        //     received -= packetSize;
        //     memcpy(readBuffer, readBuffer + packetSize, received);
        //     continue;
        // }

        TLPacket *tlPacket = NULL;
        PacketParseStatus parseStatus;
        if ((parseStatus = packetFromBytes(readBuffer, &tlPacket)) != PKT_PARSE_OK) {
            DBG_ERROR("Failed to parse packet");
            exit(1);
        }

        request = requestsSlots[tlPacket->id & 0xFFFF].request;

        WaitForSingleObject(request->mutex, INFINITE);
        request->packet = tlPacket;
        // move received packet to the destination

        // clear buffer
        // TODO handle too large PACKET_SIZE
        int packetSize = *(int*)(readBuffer + PKT_SIZE_OFFSET);
        received -= packetSize;
        memcpy(readBuffer, readBuffer + packetSize, received);
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

void sendMessageThread(void *args)
{
//     DBG_FUNC();
//     Message         *message = ((SendMessageThreadArg*)args)->message;
//     Contact         *contact = ((SendMessageThreadArg*)args)->contact;
//     SOCKET          serverSocket = ((SendMessageThreadArg*)args)->socket;
//     Request  *request = NULL;
//     int             respond;
//     TLPacket          pIn = {0}, pOut = {0};
//
//     if (contact == NULL) {
//         DBG_ERROR("Contact is null");
//         free(args);
//         return;
//     }
//     if (message == NULL) {
//         DBG_ERROR("Message is null");
//         free(args);
//         return;
//     }
//
//     deleteRequest(&request);
//     deletePacket(pIn); deletePacket(pOut);
//     request = createRequest();
//     if (request == NULL) {
//         DBG_FATAL("Failed to create request");
//         printNotification(formatError, "Can't send message");
//         free(args);
//         return;
//     }
//     pOut = createPacket(TYPE_REQUEST, CMD_MESSAGE, 0, request->id);
//     addPacketString(&pOut, contact->nickname);
//     addPacketString(&pOut, message->text);
//     sendPacket(serverSocket, pOut, &socketServerMutex);
//
//     WaitForSingleObject(request->event, INFINITE);
//     WaitForSingleObject(request->mutex, INFINITE);
//
//     pIn = packetFromBytes(request->data);
//     deleteRequest(&request);
//     if (pIn.data == NULL) {
//         DBG_ERROR("Can't create packet from respond");
//         free(args);
//         return;
//     }
//
//     respond = pIn.status;
//
//     switch (respond) {
//     case SERV_RESPOND_OK:
//         DBG_INFO("Message sent");
//         message->state = MESSAGE_SEND_SUCCESS;
//         break;
//     case SERV_RESPOND_FAILURE:
//         DBG_ERROR("Message sending failed");
//         printNotification(formatError, "Can't send message");
//         message->state = MESSAGE_SEND_FAILED;
//         break;
//     case SERV_RESPOND_NOT_FOUND:
//         DBG_ERROR("Server did not find contact");
//         message->state = MESSAGE_SEND_FAILED;
//         printNotification(formatError, "Contact is not logined");
//         break;
//     default:
//         printStatusErrorMessage(respond);
//         message->state = MESSAGE_SEND_FAILED;
//     }
//
//     SetEvent(appData.messageEvent);
//     deletePacket(pIn); deletePacket(pOut);
//     free(args);
// }
//
// static TLPacket handleNewMessage(TLPacket messagePacket)
// {
//     TLPacket respond = createPacket(TYPE_RESPOND, CMD_MESSAGE, SERV_RESPOND_FAILURE, messagePacket.id);
//     char *senderNickname, *receivedMessage;
//     size_t readPos = 0;
//     Contact *contact = contacts;
//
//     if ((senderNickname = readPacketString(&messagePacket, &readPos)) == NULL) {
//         DBG_ERROR("Can't read nickname from the message");
//         respond.status = SERV_RESPOND_CANT_PARSE;
//         return respond;
//     }
//
//     while (contact != NULL) {
//         if (strcmp(contact->nickname, senderNickname) == 0)
//             break;
//         contact = contact->next;
//     }
//     if (contact == NULL) {
//         contact = createContact(senderNickname);
//     }
//
//     if ((receivedMessage = readPacketString(&messagePacket, &readPos)) == NULL) {
//         DBG_ERROR("Can't read nickname from the message");
//         respond.status = SERV_RESPOND_CANT_PARSE;
//         return respond;
//     }
//
//     addMessage(contact, receivedMessage, false, MESSAGE_SEND_SUCCESS);
//     respond.status = SERV_RESPOND_OK;
//
//     return respond;
}

void printStatusErrorMessage(ServerRespond packetStatus)
{
    switch (packetStatus) {
    case SERV_RESPOND_FAILURE:
        DBG_ERROR("Status failed\n");
        printError("Status failed\n");
        break;
    case SERV_RESPOND_CANT_PARSE:
        DBG_WARNING("Server can't read data from the packet\n");
        printError("Server can't read data from the packet\n");
        break;
    case SERV_RESPOND_NOT_FOUND:
        DBG_WARNING("Not found\n");
        printError("Not found\n");
        break;
    case SERV_RESPOND_ALREADY_EXISTS:
        DBG_WARNING("Status already exists\n");
        printError("Status already exists\n");
        break;
    case SERV_RESPOND_SERVER_ERROR:
        DBG_WARNING("Server error\n");
        printError("Server error\n");
        break;
    case SERV_RESPOND_SIZE_MISMATCH:
        DBG_WARNING("Packet size too big\n");
        printError("Packet size too big\n");
        break;
    case SERV_RESPOND_ACTION_TO_HIMSELF:
        DBG_WARNING("Action to himself\n");
        printError("Action to himself\n");
        break;
    default:
        DBG_ERROR("Unknown respond (%d)\n", packetStatus);
        printError("Unknown respond (%d)\n", packetStatus);
    }
}