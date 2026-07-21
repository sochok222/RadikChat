#ifndef RADIKCHAT_PACKETPROCESSOR_H
#define RADIKCHAT_PACKETPROCESSOR_H

#include "packet.h"
#include "server.h"

ServerRespondPacket *process_login_packet(TlPacket *tl_packet, PerSocketContext *per_socket_context);
ServerRespond       process_create_chat_packet(CreateChatPacket *packetCreateChat);
void                process_message_packet(MessagePacket *packet_message);

#endif //RADIKCHAT_PACKETPROCESSOR_H
