#include "packet.h"
#include "debug.h"

PacketParseStatus tl_unpack_login(TlPacket *tl_packet, PacketLogin **packet_login)
{
    bool new_packet = false;
    PacketLogin *result;

    if (tl_packet != nullptr && tl_packet->command != CMD_LOGIN)
        return PKT_PARSE_INVALID_COMMAND;

    if (tl_packet == nullptr) {
        new_packet = true;
        tl_packet = alloc_tl_packet();
        tl_packet->command = CMD_LOGIN;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tl_packet = tl_packet;
    result->nickname_len = 0;
    result->nickname_capacity = 0;
    result->nickname = NULL;

    if (new_packet) {
        *packet_login = result;
        return PKT_PARSE_OK;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(PKT_F_STRING, tl_packet, &read_pos, &result->nickname)) != PKT_PARSE_OK) {
        free(result);
        return parse_status;
    }
    result->nickname_len = strlen(result->nickname);

    *packet_login = result;
    return PKT_PARSE_OK;
}

void tl_pack_login(PacketLogin *packet_login)
{
    tl_pack_data(PKT_F_STRING, packet_login->tl_packet, packet_login->nickname);
    tl_pack_header(packet_login->tl_packet);
}

void login_set_nickname(PacketLogin *packet_login, char *nickname)
{
    uint16_t nickname_len = strlen(nickname);

    if (packet_login->nickname_capacity < nickname_len + 1) {
        packet_login->nickname = realloc(packet_login->nickname, nickname_len + 1);
        if (packet_login->nickname == nullptr) {
            DBG_FATAL("Out of memory");
            exit(1);
        }
        packet_login->nickname_len = nickname_len + 1;
        packet_login->nickname_capacity = nickname_len + 1;
        memcpy(packet_login->nickname, nickname, nickname_len + 1);
    }
}

void delete_packet_login(PacketLogin *packet_login)
{
    // delete_tl_packet(packet_login->tl_packet);
    if (packet_login->nickname_capacity > 0)
        free(packet_login->nickname);
    free(packet_login);
}

PacketParseStatus tl_unpack_create_chat(TlPacket *tl_packet, PacketCreateChat **packet_create_chat)
{
    bool new_packet = false;
    PacketCreateChat *result = nullptr;

    if (tl_packet != nullptr && tl_packet->command != CMD_CREATE_CHAT)
        return PKT_PARSE_INVALID_COMMAND;

    if (tl_packet == nullptr) {
        new_packet = true;
        tl_packet = alloc_tl_packet();
        tl_packet->command = CMD_CREATE_CHAT;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tl_packet = tl_packet;
    result->receiver_id = 0;

    if (new_packet) {
        *packet_create_chat = result;
        return PKT_PARSE_OK;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(PKT_F_UINT64, tl_packet, &read_pos, &result->receiver_id)) != PKT_PARSE_OK) {
        free(result);
        return parse_status;
    }

    *packet_create_chat = result;
    return PKT_PARSE_OK;
}

void tl_pack_create_chat(PacketCreateChat *packet_create_chat)
{
    tl_pack_data(PKT_F_UINT64, packet_create_chat->tl_packet, &packet_create_chat->receiver_id);
    tl_pack_header(packet_create_chat->tl_packet);
}

void create_chat_set_receiver_id(PacketCreateChat *packet_create_chat, receiver_id receiverID)
{
    packet_create_chat->receiver_id = receiverID;
}

void delete_packet_create_chat(PacketCreateChat *packet_create_chat)
{
    // delete_tl_packet(packet_create_chat->tl_packet);
    free(packet_create_chat);
}

PacketParseStatus tl_unpack_server_respond(TlPacket *tl_packet, PacketServerRespond **packet_server_respond)
{
    bool new_packet = false;
    PacketServerRespond *result = nullptr;

    if (tl_packet != nullptr && tl_packet->command != CMD_SERVER_RESPOND)
        return PKT_PARSE_INVALID_COMMAND;

    if (tl_packet == nullptr) {
        new_packet = true;
        tl_packet = alloc_tl_packet();
        tl_packet->command = CMD_SERVER_RESPOND;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tl_packet = tl_packet;
    result->status = 0;

    if (new_packet) {
        *packet_server_respond = result;
        return PKT_PARSE_OK;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(PKT_F_UINT16, tl_packet, &read_pos, &result->status)) != PKT_PARSE_OK) {
        free(result);
        return parse_status;
    }

    *packet_server_respond = result;
    return PKT_PARSE_OK;
}

void tl_pack_server_respond(PacketServerRespond *packet_server_respond)
{
    tl_pack_data(PKT_F_UINT16, packet_server_respond->tl_packet, &packet_server_respond->status);
    tl_pack_header(packet_server_respond->tl_packet);
}

void server_respond_set_respond(PacketServerRespond *packet_server_respond, ServerRespond status)
{
    packet_server_respond->status = status;
}

void delete_packet_server_respond(PacketServerRespond *packet_server_respond)
{
    // delete_tl_packet(packet_server_respond->tl_packet);
    free(packet_server_respond);
}