#include "packetProcessor.h"
#include <networkTypes.h>
#include <debug.h>

static int sendMessage(SOCKET socket, const char *message);

void processLoginPacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         respond = PACKET_LOGIN_FAILURE,
                requestId;
    char        *nickname;
    size_t      readPos = 0;
    Packet      in, out;
    in.data = NULL; out.data = NULL;

    in = packetFromBytes(client->buffer);
    if (in.data == NULL)
        return;

    requestId = *(int*)(client->buffer + PACKET_ID_OFFSET);
    out = createPacket(PACKET_LOGIN_RESPOND, requestId);

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        respond = PACKET_ERROR_CANT_READ;
        goto sendPacket;
    }

    // Search if client with same nickname is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found same nickname\n");
            respond = PACKET_LOGIN_ALREADY_EXISTS;
            goto sendPacket;
        }
        it = it->next;
    }
    respond = PACKET_LOGIN_SUCCESS;

    // Save nickname to client
    strcpy(client->nickname, nickname);
    client->isLogined = true;

sendPacket:
    addPacketInt(&out, respond);
    sendPacket(client->socket, out, NULL);
    deletePacket(in); deletePacket(out);
}

void processCreateChatPacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         respond = PACKET_CREATE_CHAT_FAILURE,
                requestId;
    char        *nickname;
    size_t      readPos = 0;
    Packet      in, out;
    in.data = NULL; out.data = NULL;

    in = packetFromBytes(client->buffer);
    if (in.data == NULL) {
        DBG_ERROR("in.data is NULL\n");
        return;
    }

    requestId = *(int*)(client->buffer + PACKET_ID_OFFSET);
    out = createPacket(PACKET_CREATE_CHAT_RESPOND, requestId);
    if (out.data == NULL) {
        DBG_ERROR("out.data is NULL\n");
        deletePacket(in);
        return;
    }

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        respond = PACKET_ERROR_CANT_READ;
        goto sendPacket;
    }

    if (strcmp(client->nickname, nickname) == 0) {
        DBG_INFO("Client tries to create chat with himself\n");
        respond = PACKET_CREATE_CHAT_ITS_YOUR_NICK;
        goto sendPacket;
    }

    // Search for if needed client is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found needed client\n");
            respond = PACKET_CREATE_CHAT_SUCCESS;
            goto sendPacket;
        }
        it = it->next;
    }
    respond = PACKET_CREATE_CHAT_CLIENT_NOT_FOUND;

sendPacket:
    addPacketInt(&out, respond);
    sendPacket(client->socket, out, NULL);
    deletePacket(in); deletePacket(out);
}

void processMessagePacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         serverRespond = PACKET_MESSAGE_FAILURE,
                requestId,
                clientResopnd;
    char        *nickname, *message;
    size_t      readPos = 0;
    Packet      in, out;
    in.data = NULL; out.data = NULL;

    in = packetFromBytes(client->buffer);
    if (in.data == NULL) {
        DBG_ERROR("in.data is NULL\n");
        return;
    }

    requestId = *(int*)(client->buffer + PACKET_ID_OFFSET);
    out = createPacket(PACKET_MESSAGE_RESPOND, requestId);
    if (out.data == NULL) {
        DBG_ERROR("out.data is NULL\n");
        deletePacket(in);
        return;
    }

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        serverRespond = PACKET_ERROR_CANT_READ;
        goto sendPacket;
    }

    if ((message = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read message\n");
        serverRespond = PACKET_ERROR_CANT_READ;
        goto sendPacket;
    }

    // Search for if needed client is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found needed client\n");
            break;
        }
        it = it->next;
    }

    clientResopnd = sendMessage(it->socket, message);

    sendPacket:
    addPacketInt(&out, serverRespond);
    sendPacket(client->socket, out, NULL);
    deletePacket(in); deletePacket(out);
}

static int sendMessage(SOCKET socket, const char *message)
{
    Packet      in, out;
    in.data = NULL; out.data = NULL;

    out = createPacket(PACKET_MESSAGE)

    addPacketInt(&out, 0);
    addPacketString(&out, )
}