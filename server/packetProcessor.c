#include "serverUtils.h"
#include "packetProcessor.h"
#include <debug.h>
#include "server.h"
#include <tlPacket.h>

static bool send_message(ClientInfo *from, ClientInfo *to, const char *message);

PacketServerRespond *process_login_packet(TLPacket *tl_packet, PerSocketContext *per_socket_context)
{
    DBG_FUNC();

    PacketParseStatus parse_status;
    PacketLogin *packet_login;

    // Create respond packet (check of parse status is not needed because packet is created)
    PacketServerRespond *respond;
    tl_unpack_server_respond(nullptr, &respond);

    // Try to unpack login packet
    if ((parse_status = tl_unpack_login(tl_packet, &packet_login)) != PKT_PARSE_OK) {
        DBG_ERROR("Failed to parse login packet: %s", get_parse_status_string(parse_status));
        server_respond_set_respond(respond, SERV_RESPOND_CANT_PARSE);
        return respond;
    }

    // Search if client with this nickname already registered
    // TODO add mutex on client names
    PerSocketContext *it = g_clients;
    while (it != NULL) {
        if (it->nickname != NULL &&
            strcmp(it->nickname, packet_login->nickname) == 0) {
            DBG_DEBUG("Found client already registered with same nickname %s", packet_login->nickname);
            server_respond_set_respond(respond, SERV_RESPOND_ALREADY_EXISTS);
            return respond;
        }
        it = it->ctxt_forward;
    }

    // Check if nickname has enough length
    if (strlen(packet_login->nickname) == 0) {
        DBG_INFO("Nickname has 0 length");
        server_respond_set_respond(respond, SERV_RESPOND_NICKNAME_TOO_SHORT);
        return respond;
    }

    // Saving nickname
    per_socket_context->nickname = malloc(sizeof(char)*(strlen(packet_login->nickname)+1));
    strcpy(per_socket_context->nickname, packet_login->nickname);

    // free allocated data
    delete_packet_login(packet_login);

    server_respond_set_respond(respond, SERV_RESPOND_OK);
    return respond;
}

ServerRespond process_create_chat_packet(PacketCreateChat *client)
{
    return SERV_RESPOND_OK;
//     DBG_FUNC();
//     ClientInfo  *it;
//     int         requestId;
//     char        *nickname;
//     size_t      readPos = 0;
//     TLPacket      in, out;
//     in.data = NULL; out.data = NULL;
//
//     in = packet_from_bytes(client->buffer);
//     if (in.data == NULL) {
//         DBG_ERROR("in.data is NULL");
//         return;
//     }
//
//     requestId = *(int*)(client->buffer + PKT_ID_OFFSET);
//     out = createPacket(TYPE_RESPOND, CMD_CREATE_CHAT, SERV_RESPOND_FAILURE, requestId);
//     if (out.data == NULL) {
//         DBG_ERROR("out.data is NULL");
//         deletePacket(in);
//         return;
//     }
//
//     if ((nickname = readPacketString(&in, &readPos)) == NULL) {
//         DBG_ERROR("Can't read nickname");
//         out.status = SERV_RESPOND_CANT_PARSE;
//         goto send_packet;
//     }
//
//     if (strcmp(client->nickname, nickname) == 0) {
//         DBG_INFO("Client tries to create chat with himself");
//         out.status = SERV_RESPOND_ACTION_TO_HIMSELF;
//         goto send_packet;
//     }
//
//     // Search for if needed client is registered
//     it = g_ci_clients;
//     while (it != NULL) {
//         if (strcmp(it->nickname, nickname) == 0) {
//             DBG_INFO("Found needed client");
//             out.status = SERV_RESPOND_OK;
//             goto send_packet;
//         }
//         it = it->next;
//     }
//     out.status = SERV_RESPOND_NOT_FOUND;
//
// send_packet:
//     send_packet(client->socket, out, NULL);
//     deletePacket(in); deletePacket(out);
}

void process_message_packet(PacketMessage *packet_message)
{
    // DBG_FUNC();
    // ClientInfo  *it;
    // int         requestId;
    // char        *nickname, *message;
    // size_t      readPos = 0;
    // TLPacket      in, toSender;
    // in.data = NULL; toSender.data = NULL;
    //
    // in = packet_from_bytes(client->buffer);
    // if (in.data == NULL) {
    //     DBG_ERROR("in.data is NULL");
    //     if (in.parseError == PARSE_ERROR_MALLOC_FAILED) {
    //         DBG_ERROR("Can't allocate memory");
    //     } else {
    //         DBG_ERROR("Wrong size of data");
    //     }
    //     return;
    // }
    //
    // requestId = *(int*)(client->buffer + PKT_ID_OFFSET);
    // toSender = createPacket(TYPE_RESPOND, CMD_MESSAGE, SERV_RESPOND_FAILURE, requestId);
    //
    // if ((nickname = readPacketString(&in, &readPos)) == NULL) {
    //     DBG_ERROR("Can't read nickname");
    //     toSender.status = SERV_RESPOND_CANT_PARSE;
    //     send_packet(client->socket, toSender, NULL);
    //     deletePacket(in); deletePacket(toSender);
    //     return;
    // }
    //
    // if ((message = readPacketString(&in, &readPos)) == NULL) {
    //     DBG_ERROR("Can't read message");
    //     toSender.status = SERV_RESPOND_CANT_PARSE;
    //     send_packet(client->socket, toSender, NULL);
    //     deletePacket(in); deletePacket(toSender);
    //     return;
    // }
    //
    // // Search for if needed client is registered
    // it = g_ci_clients;
    // while (it != NULL) {
    //     if (strcmp(it->nickname, nickname) == 0) {
    //         DBG_INFO("Found needed client");
    //         break;
    //     }
    //     it = it->next;
    // }
    // if (it == NULL) {
    //     DBG_ERROR("Can't find client\n");
    //     toSender.status = SERV_RESPOND_NOT_FOUND;
    // }
    //
    // if (it != NULL && send_message(client, it, message) == true) {
    //     DBG_INFO("Message sent successfully\n");
    //     toSender.status = SERV_RESPOND_OK;
    // }
    // else {
    //     DBG_INFO("Failed to send message\n");
    //     toSender.status = SERV_RESPOND_FAILURE;
    // }
    //
    // send_packet(client->socket, toSender, NULL);
    // deletePacket(in); deletePacket(toSender);
}

// TODO add packet resending if send fails
static bool send_message(ClientInfo *from, ClientInfo *to, const char *message)
{
    // TLPacket      in, out;
    // fd_set      fdRespond;
    // int         cycle, totalReceived = 0;
    // char        buffer[100];
    // in.data = NULL; out.data = NULL;
    //
    // // Status and id are ignored
    // out = createPacket(TYPE_DELIVERY, CMD_MESSAGE, SERV_RESPOND_OK, 0);
    //
    // addPacketString(&out, from->nickname);
    // addPacketString(&out, message);
    //
    // send_packet(to->socket, out, NULL);
    // deletePacket(out);
    //
    // // FIXME: this cycle method is unreliable, add handling to select timeout
    // for (cycle = 0; cycle < 5; cycle++) {
    //     FD_ZERO(&fdRespond);
    //     FD_SET(to->socket, &fdRespond);
    //     if (select(0, &fdRespond, NULL, NULL, NULL) < 0) {
    //         DBG_FATAL("select() failed.\n");
    //         log_wsa_error(WSAGetLastError());
    //         return false;
    //     }
    //
    //     if (FD_ISSET(to->socket, &fdRespond)) {
    //         int received;
    //         received = recv(to->socket, buffer + totalReceived, sizeof(buffer) - totalReceived, 0);
    //
    //         if (received <= 0 && errno != EAGAIN) {
    //             DBG_DEBUG("Disconnect from %s client\n", get_client_address(to));
    //             delete_client(to);
    //             break;
    //         }
    //         totalReceived += received;
    //
    //         if (totalReceived >= PKT_HEADER_SIZE) {
    //             // Expecting only header, without payload
    //             if (*(size_t*)(buffer + PKT_SIZE_OFFSET) > PKT_HEADER_SIZE) {
    //                 return false;
    //             }
    //             return *(int*)(buffer + PACKET_STATUS_OFFSET) == SERV_RESPOND_OK;
    //         }
    //     } else {
    //         continue;
    //     }
    // }
    //
    // return false;
}