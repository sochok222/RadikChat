#include "packet.h"
#include "debug.h"

#define FIELD_TYPE(field) _Generic((field), \
    const char*: PKT_F_STRING, \
    char *:     PKT_F_STRING, \
    uint8_t:    PKT_F_UINT8, \
    uint16_t:   PKT_F_UINT16, \
    uint32_t:   PKT_F_UINT32, \
    uint64_t:   PKT_F_UINT64, \
    int8_t:     PKT_F_INT8, \
    int16_t:    PKT_F_INT16, \
    int32_t:    PKT_F_INT32, \
    int64_t:    PKT_F_INT64)

LoginPacket *allocate_login_packet()
{
    LoginPacket *new_packet = malloc(sizeof(*new_packet));
    if (new_packet == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }

    new_packet->base.packet_id      = 0;
    new_packet->nickname            = nullptr;
    new_packet->nickname_capacity   = 0;
    new_packet->nickname_len        = 0;
    return new_packet;
}

TlPacket *serialize_login_packet(const LoginPacket *login_packet)
{
    TlPacket *tl_packet = allocate_tl_packet();
    tl_packet->command = CMD_LOGIN;
    tl_packet->id = login_packet->base.packet_id;
    if (login_packet->nickname != nullptr)
        tl_pack_data(PKT_F_STRING, tl_packet, login_packet->nickname);
    else {
        DBG_WARNING("login_packet->nickname is nullptr");
    }
    tl_pack_header(tl_packet);

    return tl_packet;
}

PacketParseStatus deserialize_login_packet(const TlPacket *tl_packet, LoginPacket **login_packet)
{
    LoginPacket *result = allocate_login_packet();

    if (tl_packet->command != CMD_LOGIN) {
        DBG_ERROR("Invalid command");
        delete_login_packet(result);
        return PKT_PARSE_INVALID_COMMAND;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(FIELD_TYPE(result->nickname), tl_packet, &read_pos, &result->nickname, result->nickname_capacity)) != PKT_PARSE_OK) {
        delete_login_packet(result);
        return parse_status;
    }
    result->nickname_len = strlen(result->nickname);

    result->base.packet_id = tl_packet->id;

    *login_packet = result;
    return PKT_PARSE_OK;
}

void delete_login_packet(LoginPacket *login_packet)
{
    if (login_packet == nullptr) {
        DBG_FATAL("Passed nullptr to free()");
        exit(1);
    }
    if (login_packet->nickname != nullptr)
        free(login_packet->nickname);
    free(login_packet);
}

void login_packet_set_nickname(LoginPacket *login_packet, const char *nickname)
{
    const uint16_t nickname_len = strlen(nickname);

    if (login_packet->nickname_capacity < nickname_len + 1) {
        login_packet->nickname = realloc(login_packet->nickname, nickname_len + 1);
        if (login_packet->nickname == nullptr) {
            DBG_FATAL("Out of memory");
            exit(1);
        }
        login_packet->nickname_capacity = nickname_len + 1;
    }
    login_packet->nickname_len = nickname_len + 1;
    memcpy(login_packet->nickname, nickname, nickname_len + 1);
}

CreateChatPacket *allocate_create_chat_packet()
{
    CreateChatPacket *new_packet = malloc(sizeof(*new_packet));
    if (new_packet == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }

    new_packet->base.packet_id  = 0;
    new_packet->receiver_id     = 0;
    return new_packet;
}
TlPacket *serialize_create_chat_packet(const CreateChatPacket *create_chat_packet)
{
    TlPacket *tl_packet = allocate_tl_packet();
    tl_packet->command = CMD_CREATE_CHAT;
    tl_packet->id = create_chat_packet->base.packet_id;
    tl_pack_data(FIELD_TYPE(create_chat_packet->receiver_id), tl_packet, &create_chat_packet->receiver_id);
    tl_pack_header(tl_packet);
    return tl_packet;
}

PacketParseStatus deserialize_create_chat_packet(const TlPacket *tl_packet, CreateChatPacket **create_chat_packet)
{
    CreateChatPacket *result = allocate_create_chat_packet();

    if (tl_packet->command != CMD_CREATE_CHAT) {
        DBG_ERROR("Invalid command");
        delete_create_chat_packet(result);
        return PKT_PARSE_INVALID_COMMAND;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(FIELD_TYPE(result->receiver_id),
        tl_packet, &read_pos, &result->receiver_id, sizeof(result->receiver_id))) != PKT_PARSE_OK) {
        free(result);
        return parse_status;
    }

    result->base.packet_id = tl_packet->id;

    *create_chat_packet = result;
    return PKT_PARSE_OK;
}

void delete_create_chat_packet(CreateChatPacket *create_chat_packet)
{
    if (create_chat_packet == nullptr) {
        DBG_FATAL("passed nullptr to free()");
        exit(1);
    }
    free(create_chat_packet);
}

ServerRespondPacket *allocate_server_respond_packet()
{
    ServerRespondPacket *new_packet = malloc(sizeof(*new_packet));
    if (new_packet == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }

    new_packet->base.packet_id  = 0;
    new_packet->status          = SERVER_RESPOND_OK;
    return new_packet;
}

TlPacket *serialize_server_respond_packet(const ServerRespondPacket *server_respond_packet)
{
    TlPacket *tl_packet = allocate_tl_packet();
    tl_packet->command = CMD_SERVER_RESPOND;
    tl_packet->id = server_respond_packet->base.packet_id;
    tl_pack_data(FIELD_TYPE(server_respond_packet->status), tl_packet, &server_respond_packet->status);
    tl_pack_header(tl_packet);
    return tl_packet;
}

PacketParseStatus deserialize_server_respond_packet(const TlPacket *tl_packet, ServerRespondPacket **server_respond_packet)
{
    ServerRespondPacket *result = allocate_server_respond_packet();

    if (tl_packet->command != CMD_SERVER_RESPOND) {
        DBG_ERROR("Invalid command");
        delete_server_respond_packet(result);
        return PKT_PARSE_INVALID_COMMAND;
    }

    uint32_t read_pos = 0; int parse_status;
    if ((parse_status = read_packet_field(FIELD_TYPE(result->status), tl_packet, &read_pos, &result->status, sizeof(result->status))) != PKT_PARSE_OK) {
        free(result);
        return parse_status;
    }

    result->base.packet_id = tl_packet->id;

    *server_respond_packet = result;
    return PKT_PARSE_OK;
}
void delete_server_respond_packet(ServerRespondPacket *server_respond_packet)
{
    if (server_respond_packet == nullptr) {
        DBG_FATAL("passed nullptr to free()");
        exit(1);
    }
    free(server_respond_packet);
}
