#include "serverUtils.h"
#include <debug.h>
#include <packetManager/packet.h>

static bool sendMessage(ClientInfo *from, ClientInfo *to, const char *message);

void processLoginPacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         requestId;
    char        *nickname;
    size_t      readPos = 0;
    Packet      in, out;
    in.data = NULL; out.data = NULL;

    in = packetFromBytes(client->buffer);
    if (in.data == NULL)
        return;

    requestId = *(int*)(client->buffer + PACKET_ID_OFFSET);
    out = createPacket(TYPE_RESPOND, COMMAND_LOGIN, STATUS_FAILURE, requestId);

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        out.status = STATUS_CANT_READ;
        goto sendPacket;
    }

    // Search if client with same nickname is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found same nickname\n");
            out.status = STATUS_ALREADY_EXISTS;
            goto sendPacket;
        }
        it = it->next;
    }
    out.status = STATUS_OK;

    // Save nickname to client
    strcpy(client->nickname, nickname);
    client->isLogined = true;

sendPacket:
    sendPacket(client->socket, out, NULL);
    deletePacket(in); deletePacket(out);
}

void processCreateChatPacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         requestId;
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
    out = createPacket(TYPE_RESPOND, COMMAND_CREATE_CHAT, STATUS_FAILURE, requestId);
    if (out.data == NULL) {
        DBG_ERROR("out.data is NULL\n");
        deletePacket(in);
        return;
    }

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        out.status = STATUS_CANT_READ;
        goto sendPacket;
    }

    if (strcmp(client->nickname, nickname) == 0) {
        DBG_INFO("Client tries to create chat with himself\n");
        out.status = STATUS_ACTION_TO_HIMSELF;
        goto sendPacket;
    }

    // Search for if needed client is registered
    it = clients;
    while (it != NULL) {
        if (strcmp(it->nickname, nickname) == 0) {
            DBG_INFO("Found needed client\n");
            out.status = STATUS_OK;
            goto sendPacket;
        }
        it = it->next;
    }
    out.status = STATUS_NOT_FOUND;

sendPacket:
    sendPacket(client->socket, out, NULL);
    deletePacket(in); deletePacket(out);
}

void processMessagePacket(ClientInfo *client)
{
    DBG_FUNC();
    ClientInfo  *it;
    int         requestId;
    bool        toRespond;
    char        *nickname, *message;
    size_t      readPos = 0;
    Packet      in, toSender;
    in.data = NULL; toSender.data = NULL;

    in = packetFromBytes(client->buffer);
    if (in.data == NULL) {
        DBG_ERROR("in.data is NULL\n");
        if (in.parseError == PARSE_ERROR_MALLOC_FAILED) {
            DBG_ERROR("Can't allocate memory\n");
        } else {
            DBG_ERROR("Wrong size of data\n");
        }
        return;
    }

    requestId = *(int*)(client->buffer + PACKET_ID_OFFSET);
    toSender = createPacket(TYPE_RESPOND, COMMAND_MESSAGE, STATUS_FAILURE, requestId);

    if ((nickname = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read nickname\n");
        toSender.status = STATUS_CANT_READ;
        sendPacket(client->socket, toSender, NULL);
        deletePacket(in); deletePacket(toSender);
        return;
    }

    if ((message = readPacketString(&in, &readPos)) == NULL) {
        DBG_ERROR("Can't read message\n");
        toSender.status = STATUS_CANT_READ;
        sendPacket(client->socket, toSender, NULL);
        deletePacket(in); deletePacket(toSender);
        return;
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

    if (sendMessage(client, it, message) == true)
        toSender.status = STATUS_OK;
    else
        toSender.status = STATUS_FAILURE;

    sendPacket(client->socket, toSender, NULL);
    deletePacket(in); deletePacket(toSender);
}

// TODO add packet resending if send fails
static bool sendMessage(ClientInfo *from, ClientInfo *to, const char *message)
{
    Packet      in, out;
    fd_set      fdRespond;
    int         cycle, totalReceived = 0;
    char        buffer[100];
    in.data = NULL; out.data = NULL;
    TIMEVAL timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // Status and id are ignored
    out = createPacket(TYPE_DELIVERY, COMMAND_MESSAGE, STATUS_OK, 0);

    addPacketString(&out, from->nickname);
    addPacketString(&out, message);

    sendPacket(to->socket, out, NULL);

    for (cycle = 0; cycle < 3; cycle++) {
        FD_ZERO(&fdRespond);
        FD_SET(to->socket, &fdRespond);
        if (select(0, &fdRespond, NULL, NULL, &timeout) < 0) {
            DBG_FATAL("select() failed.\n");
            logWsaError(WSAGetLastError());
            return false;
        }

        if (FD_ISSET(to->socket, &fdRespond)) {
            int received;
            received = recv(to->socket, buffer + totalReceived, sizeof(buffer) - totalReceived, 0);

            if (received <= 0 && errno != EAGAIN) {
                DBG_DEBUG("Disconnect from %s client\n", getClientAddress(to));
                deleteClient(to);
                break;
            }
            totalReceived += received;

            if (totalReceived >= PACKET_HEADER_SIZE) {
                // Expecting only header, without payload
                if (*(size_t*)(buffer + PACKET_SIZE_OFFSET) > PACKET_HEADER_SIZE) {
                    return false;
                }
                return *(int*)(buffer + PACKET_STATUS_OFFSET) == STATUS_OK;
            }
        } else {
            return false;
        }
    }

    return false;
}