#ifndef RADIKCHAT_PACKETPROCESSOR_H
#define RADIKCHAT_PACKETPROCESSOR_H

#include "packet.h"
#include "server.h"

PacketServerRespond *process_login_packet(TLPacket *tl_packet, PerSocketContext *per_socket_context);
ServerRespond       process_create_chat_packet(PacketCreateChat *packetCreateChat);
void                process_message_packet(PacketMessage *packet_message);

#endif //RADIKCHAT_PACKETPROCESSOR_H
