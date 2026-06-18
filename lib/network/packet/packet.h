#ifndef RADIKCHAT_PACKET_H
#define RADIKCHAT_PACKET_H
#include "tlPacket.h"

typedef uint64_t ReceiverID;

typedef struct sPacketLogin
{
    TLPacket    *childPacket;
    uint16_t    nicknameLen; // len + null char
    uint16_t    nicknameCapacity;
    char        *nickname;
} PacketLogin;

typedef struct sPacketCreateChat
{
    TLPacket *childPacket;
    ReceiverID receiverID;
} PacketCreateChat;

typedef struct sPacketServerRespond
{
    TLPacket        *childPacket;
    ServerRespond   status;
} PacketServerRespond;

typedef struct sPacketMessage
{
    TLPacket    *childPacket;
    uint8_t     toChat;
    ReceiverID  receiverID;
    size_t      messageLen;
    char        *message;
} PacketMessage;

// * Unpack - allocate new typed packet and assign transport-layer packet as child to it (tl is freed with typed packet).
// If packet is nullptr means that it will be allocated too.
// * Pack   - place all typed packet data into tl-packet's payload
PacketParseStatus tlUnpackLogin(TLPacket *tlPacket, PacketLogin **packetLogin);
inline void tlPackLogin(PacketLogin *packetLogin);
void loginSetNickname(PacketLogin *packetLogin, char *nickname);
inline void deletePacketLogin(PacketLogin *packetLogin);

PacketParseStatus tlUnpackCreateChat(TLPacket *tlPacket, PacketCreateChat **packetCreateChat);
inline void tlPackCreateChat(PacketCreateChat *packetCreateChat);
inline void createChatSetReceiverID(PacketCreateChat *packetCreateChat, ReceiverID receiverID);
inline void deletePacketCreateChat(PacketCreateChat *packetCreateChat);

PacketParseStatus tlUnpackServerRespond(TLPacket *tlPacket, PacketServerRespond **packetServerRespond);
inline void tlPackServerRespond(PacketServerRespond *packetServerRespond);
inline void serverRespondSetRespond(PacketServerRespond *packetServerRespond, uint16_t status);
inline void deletePacketServerRespond(PacketServerRespond *packetServerRespond);

#endif //RADIKCHAT_PACKET_H
