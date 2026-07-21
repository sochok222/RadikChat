#include "packet_processor.h"
#include "server.h"
#include <debug.h>
#include <tl_packet.h>

ServerRespondPacket *process_login_packet(TlPacket *tl_packet, PerSocketContext *per_socket_context)
{
    DBG_FUNC();

    ServerRespondPacket *respond_packet = allocate_server_respond_packet();
    respond_packet->base.packet_id = tl_packet->id;

    // Try to deserialize login packet
    LoginPacket *login_packet;
    PacketParseStatus parse_status;
    if ((parse_status = deserialize_login_packet(tl_packet, &login_packet)) != PKT_PARSE_OK) {
        DBG_ERROR("Failed to parse login packet: %s", get_parse_status_string(parse_status));
        respond_packet->status = SERVER_RESPOND_CANT_PARSE;
        return respond_packet;
    }

    // Search if client with this nickname already registered
    // TODO add mutex on client names
    PerSocketContext *it = g_clients;
    while (it != NULL) {
        if (it->nickname != NULL &&
            strcmp(it->nickname, login_packet->nickname) == 0) {
            DBG_DEBUG("Found client already registered with same nickname %s", login_packet->nickname);
            respond_packet->status = SERVER_RESPOND_ALREADY_EXISTS;

            delete_login_packet(login_packet);
            return respond_packet;
        }
        it = it->ctxt_forward;
    }

    // Check if nickname has enough length
    if (strlen(login_packet->nickname) == 0) {
        DBG_INFO("Nickname has 0 length");
        respond_packet->status =  SERVER_RESPOND_NICKNAME_TOO_SHORT;

        delete_login_packet(login_packet);
        return respond_packet;
    }

    // Saving nickname
    per_socket_context->nickname = malloc(sizeof(char)*(strlen(login_packet->nickname)+1));
    strcpy(per_socket_context->nickname, login_packet->nickname);

    // free allocated data
    delete_login_packet(login_packet);

    respond_packet->status = SERVER_RESPOND_OK;
    return respond_packet;
}

ServerRespond process_create_chat_packet(CreateChatPacket *client)
{
    return SERVER_RESPOND_OK;
//     DBG_FUNC();
//     ClientInfo  *it;
//     int         requestId;
//     char        *nickname;
//     size_t      readPos = 0;
//     TlPacket      in, out;
//     in.data = NULL; out.data = NULL;
//
//     in = packet_from_raw_data(client->buffer);
//     if (in.data == NULL) {
//         DBG_ERROR("in.data is NULL");
//         return;
//     }
//
//     requestId = *(int*)(client->buffer + PKT_ID_OFFSET);
//     out = createPacket(TYPE_RESPOND, CMD_CREATE_CHAT, SERVER_RESPOND_FAILURE, requestId);
//     if (out.data == NULL) {
//         DBG_ERROR("out.data is NULL");
//         deletePacket(in);
//         return;
//     }
//
//     if ((nickname = readPacketString(&in, &readPos)) == NULL) {
//         DBG_ERROR("Can't read nickname");
//         out.status = SERVER_RESPOND_CANT_PARSE;
//         goto send_packet;
//     }
//
//     if (strcmp(client->nickname, nickname) == 0) {
//         DBG_INFO("Client tries to create chat with himself");
//         out.status = SERVER_RESPOND_ACTION_TO_HIMSELF;
//         goto send_packet;
//     }
//
//     // Search for if needed client is registered
//     it = g_ci_clients;
//     while (it != NULL) {
//         if (strcmp(it->nickname, nickname) == 0) {
//             DBG_INFO("Found needed client");
//             out.status = SERVER_RESPOND_OK;
//             goto send_packet;
//         }
//         it = it->next;
//     }
//     out.status = SERVER_RESPOND_NOT_FOUND;
//
// send_packet:
//     send_packet(client->socket, out, NULL);
//     deletePacket(in); deletePacket(out);
}

void process_message_packet(MessagePacket *packet_message)
{
    // DBG_FUNC();
    // ClientInfo  *it;
    // int         requestId;
    // char        *nickname, *message;
    // size_t      readPos = 0;
    // TlPacket      in, toSender;
    // in.data = NULL; toSender.data = NULL;
    //
    // in = packet_from_raw_data(client->buffer);
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
    // toSender = createPacket(TYPE_RESPOND, CMD_MESSAGE, SERVER_RESPOND_FAILURE, requestId);
    //
    // if ((nickname = readPacketString(&in, &readPos)) == NULL) {
    //     DBG_ERROR("Can't read nickname");
    //     toSender.status = SERVER_RESPOND_CANT_PARSE;
    //     send_packet(client->socket, toSender, NULL);
    //     deletePacket(in); deletePacket(toSender);
    //     return;
    // }
    //
    // if ((message = readPacketString(&in, &readPos)) == NULL) {
    //     DBG_ERROR("Can't read message");
    //     toSender.status = SERVER_RESPOND_CANT_PARSE;
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
    //     toSender.status = SERVER_RESPOND_NOT_FOUND;
    // }
    //
    // if (it != NULL && send_message(client, it, message) == true) {
    //     DBG_INFO("Message sent successfully\n");
    //     toSender.status = SERVER_RESPOND_OK;
    // }
    // else {
    //     DBG_INFO("Failed to send message\n");
    //     toSender.status = SERVER_RESPOND_FAILURE;
    // }
    //
    // send_packet(client->socket, toSender, NULL);
    // deletePacket(in); deletePacket(toSender);
}