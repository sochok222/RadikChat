#ifndef RADIKCHAT_PACKET_H
#define RADIKCHAT_PACKET_H
#include "tl_packet.h"

typedef uint64_t receiver_id;

typedef struct sPacketLogin
{
    TlPacket    *tl_packet;
    uint16_t    nickname_len; // len + null char
    uint16_t    nickname_capacity;
    char        *nickname;
} PacketLogin;

typedef struct sPacketCreateChat
{
    TlPacket *tl_packet;
    receiver_id receiver_id;
} PacketCreateChat;

typedef struct sPacketServerRespond
{
    TlPacket        *tl_packet;
    ServerRespond   status;
} PacketServerRespond;

typedef struct sPacketMessage
{
    TlPacket    *tl_packet;
    uint8_t     to_chat;
    receiver_id receiver_id;
    size_t      message_len;
    char        *message;
} PacketMessage;

// * Unpack - allocate new typed packet and assign transport-layer packet as child to it (tl is freed with typed packet).
// If packet is nullptr means that it will be allocated too.
// * Pack   - place all typed packet data into tl-packet's payload
PacketParseStatus tl_unpack_login(TlPacket *tl_packet, PacketLogin **packet_login);
#define create_login_packet(packet_login) (tl_unpack_login(NULL, packet_login))
inline void tl_pack_login(PacketLogin *packet_login);
void login_set_nickname(PacketLogin *packet_login, char *nickname);
inline void delete_packet_login(PacketLogin *packet_login);

PacketParseStatus tl_unpack_create_chat(TlPacket *tl_packet, PacketCreateChat **packet_create_chat);
#define create_create_chat_packet(packet_create_chat) (tl_unpack_create_chat(NULL, packet_create_chat))
inline void tl_pack_create_chat(PacketCreateChat *packet_create_chat);
inline void create_chat_set_receiver_id(PacketCreateChat *packet_create_chat, receiver_id receiverID);
inline void delete_packet_create_chat(PacketCreateChat *packet_create_chat);

PacketParseStatus tl_unpack_server_respond(TlPacket *tl_packet, PacketServerRespond **packet_server_respond);
#define create_server_respond_packet(packet_server_respond) (tl_unpack_server_respond(packet_server_respond, NULL))
inline void tl_pack_server_respond(PacketServerRespond *packet_server_respond);
inline void server_respond_set_respond(PacketServerRespond *packet_server_respond, uint16_t status);
inline void delete_packet_server_respond(PacketServerRespond *packet_server_respond);

#endif //RADIKCHAT_PACKET_H
