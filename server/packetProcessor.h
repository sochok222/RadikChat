#ifndef RADIKCHAT_PACKETPROCESSOR_H
#define RADIKCHAT_PACKETPROCESSOR_H

#include "serverUtils.h"
#include "server.h"
#include "packet.h"

PacketServerRespond *processLoginPacket(TLPacket *tlPacket, PerSocketContext *perSocketContext);
ServerRespond   processCreateChatPacket(PacketCreateChat *packetCreateChat);
void            processMessagePacket(PacketMessage *packetMessage);

#endif //RADIKCHAT_PACKETPROCESSOR_H
