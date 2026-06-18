#ifndef RADIKCHAT_PACKETPROCESSOR_H
#define RADIKCHAT_PACKETPROCESSOR_H

#include "serverUtils.h"
#include "server.h"
#include "packet.h"

void processLoginPacket(ClientInfo *client);
void processCreateChatPacket(ClientInfo *client);
void processMessagePacket(ClientInfo *client);

#endif //RADIKCHAT_PACKETPROCESSOR_H
