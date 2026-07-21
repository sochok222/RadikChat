#include "client_utils.h"

#include "chats_manager.h"
#include "client.h"
#include "console_output.h"
#include "contacts_manager.h"
#include "request.h"
#include "tl_packet.h"

#include <debug.h>
#include <process.h>
#include <ws2tcpip.h>

#define MAX_READ_BUFFER_SIZE 1024

static fd_set fd_clients;

HANDLE socket_server_mutex = INVALID_HANDLE_VALUE;
HANDLE notifications_semaphore = INVALID_HANDLE_VALUE;
HANDLE notification_thread_run_mutex = INVALID_HANDLE_VALUE;

static TlPacket handle_new_message(TlPacket p);

void init_client_utils()
{
    notification_thread_run_mutex = CreateMutex(NULL, FALSE, NULL);
    FD_ZERO(&fd_clients);
}

fd_set wait_for_sever_respond(SOCKET server, struct timeval *timeout)
{
    fd_set fd_reads;
    if (!FD_ISSET(server, &fd_clients))
        FD_SET(server, &fd_clients);

    fd_reads = fd_clients;

    if (select(0, &fd_reads, 0, 0, timeout) < 0) {
        DBG_FATAL("select() failed.");
        log_wsa_error(WSAGetLastError());
        exit(1); // TODO: add proper error handling
    }
    return fd_reads;
}

void socket_thread(void*)
{
    int     received = 0;
    uint8_t read_buffer[MAX_READ_BUFFER_SIZE];
    notifications_semaphore = CreateSemaphore(NULL, 0, 100, NULL);
    Request *request = NULL;

    while (WaitForSingleObject(socket_thread_run_mutex, 75L) == WAIT_TIMEOUT) {
        wait_for_sever_respond(socket_server, 0);

        received += recv(socket_server, (char*)read_buffer + received, MAX_READ_BUFFER_SIZE - received, 0);

        if (received == -1) {
            log_wsa_error(WSAGetLastError());
        }
        if (received == MAX_READ_BUFFER_SIZE) {
            // TODO handle this
        }

        if (received < 4)
            continue;

        if (*(int*)(read_buffer + PKT_SIZE_OFFSET) > received)
            continue;

        // if (*(int*)(readBuffer + PACKET_TYPE_OFFSET) == TYPE_DELIVERY) {
        //     TlPacket notification, respond;
        //     DBG_INFO("Received a notification");
        //     notification = packet_from_bytes(readBuffer);
        //     respond = handle_new_message(notification);
        //     send_packet(socket_server, respond, &socket_server_mutex);
        //     deletePacket(notification);
        //     deletePacket(respond);
        //
        //     int packetSize = *(int*)(readBuffer + PKT_SIZE_OFFSET);
        //     received -= packetSize;
        //     memcpy(readBuffer, readBuffer + packetSize, received);
        //     continue;
        // }

        TlPacket *tl_packet = NULL;
        PacketParseStatus parse_status;
        if ((parse_status = packet_from_bytes(read_buffer, &tl_packet)) != PKT_PARSE_OK) {
            DBG_ERROR("Failed to parse packet (%d)", parse_status);
            exit(1);
        }

        request = g_requests_slots[tl_packet->id & 0xFFFF].request;

        WaitForSingleObject(request->mutex, INFINITE);
        request->packet = tl_packet;
        // move received packet to the destination

        // clear buffer
        // TODO handle too large PACKET_SIZE
        int packet_size = *(int*)(read_buffer + PKT_SIZE_OFFSET);
        received -= packet_size;
        memcpy(read_buffer, read_buffer + packet_size, received);
        SetEvent(request->event);
        ReleaseMutex(request->mutex);
    }
    _endthread();
}

void send_message_thread(void *args)
{
//     DBG_FUNC();
//     Message         *message = ((SendMessageThreadArg*)args)->message;
//     Contact         *contact = ((SendMessageThreadArg*)args)->contact;
//     SOCKET          serverSocket = ((SendMessageThreadArg*)args)->socket;
//     Request  *request = NULL;
//     int             respond;
//     TlPacket          pIn = {0}, pOut = {0};
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
//     delete_request(&request);
//     deletePacket(pIn); deletePacket(pOut);
//     request = create_request();
//     if (request == NULL) {
//         DBG_FATAL("Failed to create request");
//         print_notification(formatError, "Can't send message");
//         free(args);
//         return;
//     }
//     pOut = createPacket(TYPE_REQUEST, CMD_MESSAGE, 0, request->id);
//     addPacketString(&pOut, contact->nickname);
//     addPacketString(&pOut, message->text);
//     send_packet(serverSocket, pOut, &socket_server_mutex);
//
//     WaitForSingleObject(request->event, INFINITE);
//     WaitForSingleObject(request->mutex, INFINITE);
//
//     pIn = packet_from_bytes(request->data);
//     delete_request(&request);
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
//         print_notification(formatError, "Can't send message");
//         message->state = MESSAGE_SEND_FAILED;
//         break;
//     case SERV_RESPOND_NOT_FOUND:
//         DBG_ERROR("Server did not find contact");
//         message->state = MESSAGE_SEND_FAILED;
//         print_notification(formatError, "Contact is not logined");
//         break;
//     default:
//         print_status_error_message(respond);
//         message->state = MESSAGE_SEND_FAILED;
//     }
//
//     SetEvent(app_data.message_event);
//     deletePacket(pIn); deletePacket(pOut);
//     free(args);
// }
//
// static TlPacket handle_new_message(TlPacket messagePacket)
// {
//     TlPacket respond = createPacket(TYPE_RESPOND, CMD_MESSAGE, SERV_RESPOND_FAILURE, messagePacket.id);
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
//         contact = create_contact(senderNickname);
//     }
//
//     if ((receivedMessage = readPacketString(&messagePacket, &readPos)) == NULL) {
//         DBG_ERROR("Can't read nickname from the message");
//         respond.status = SERV_RESPOND_CANT_PARSE;
//         return respond;
//     }
//
//     add_message(contact, receivedMessage, false, MESSAGE_SEND_SUCCESS);
//     respond.status = SERV_RESPOND_OK;
//
//     return respond;
}

void print_status_error_message(ServerRespond packet_status)
{
    switch (packet_status) {
    case SERV_RESPOND_FAILURE:
        DBG_ERROR("Status failed\n");
        print_error("Status failed\n");
        break;
    case SERV_RESPOND_CANT_PARSE:
        DBG_WARNING("Server can't read data from the packet\n");
        print_error("Server can't read data from the packet\n");
        break;
    case SERV_RESPOND_NOT_FOUND:
        DBG_WARNING("Not found\n");
        print_error("Not found\n");
        break;
    case SERV_RESPOND_ALREADY_EXISTS:
        DBG_WARNING("Status already exists\n");
        print_error("Status already exists\n");
        break;
    case SERV_RESPOND_SERVER_ERROR:
        DBG_WARNING("Server error\n");
        print_error("Server error\n");
        break;
    case SERV_RESPOND_SIZE_MISMATCH:
        DBG_WARNING("Packet size too big\n");
        print_error("Packet size too big\n");
        break;
    case SERV_RESPOND_ACTION_TO_HIMSELF:
        DBG_WARNING("Action to himself\n");
        print_error("Action to himself\n");
        break;
    default:
        DBG_ERROR("Unknown respond (%d)\n", packet_status);
        print_error("Unknown respond (%d)\n", packet_status);
    }
}