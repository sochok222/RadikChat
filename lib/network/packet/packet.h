#ifndef RADIKCHAT_PACKET_H
#define RADIKCHAT_PACKET_H
#include "tl_packet.h"

typedef uint64_t receiverId;

typedef struct PacketBase
{
    TlPacketId packet_id;
} PacketBase;

typedef struct LoginPacket
{
    PacketBase  base;
    uint16_t    nickname_len; // len + null char
    uint16_t    nickname_capacity;
    char        *nickname;
} LoginPacket;

typedef struct CreateChatPacket
{
    PacketBase  base;
    receiverId  receiver_id;
} CreateChatPacket;

typedef struct ServerRespondPacket
{
    PacketBase      base;
    ServerRespond   status;
} ServerRespondPacket;

typedef struct MessagePacket
{
    PacketBase  base;
    uint8_t     to_chat;
    receiverId  receiver_id;
    size_t      message_len;
    char        *message;
} MessagePacket;

LoginPacket         *allocate_login_packet();
TlPacket            *serialize_login_packet(const LoginPacket *login_packet);
PacketParseStatus   deserialize_login_packet(const TlPacket *tl_packet, LoginPacket **login_packet);
void                delete_login_packet(LoginPacket *login_packet);
void                login_packet_set_nickname(LoginPacket *login_packet, const char *nickname);

CreateChatPacket    *allocate_create_chat_packet();
TlPacket            *serialize_create_chat_packet(const CreateChatPacket *create_chat_packet);
PacketParseStatus   deserialize_create_chat_packet(const TlPacket *tl_packet, CreateChatPacket **create_chat_packet);
void                delete_create_chat_packet(CreateChatPacket *create_chat_packet);

ServerRespondPacket *allocate_server_respond_packet();
TlPacket            *serialize_server_respond_packet(const ServerRespondPacket *server_respond_packet);
PacketParseStatus   deserialize_server_respond_packet(const TlPacket *tl_packet, ServerRespondPacket **server_respond_packet);
void                delete_server_respond_packet(ServerRespondPacket *server_respond_packet);

#endif //RADIKCHAT_PACKET_H
