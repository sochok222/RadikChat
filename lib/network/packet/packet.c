#include "packet.h"
#include "debug.h"

PacketParseStatus tlUnpackLogin(TLPacket *tlPacket, PacketLogin **packetLogin)
{
    bool newPacket = false;
    PacketLogin *result;

    if (tlPacket != nullptr && tlPacket->command != CMD_LOGIN)
        return PKT_PARSE_INVALID_COMMAND;

    if (tlPacket == nullptr) {
        newPacket = true;
        tlPacket = allocTLPacket();
        tlPacket->command = CMD_LOGIN;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tlPacket = tlPacket;
    result->nicknameLen = 0;
    result->nicknameCapacity = 0;
    result->nickname = NULL;

    if (newPacket) {
        *packetLogin = result;
        return PKT_PARSE_OK;
    }

    uint32_t readPos = 0; int parseStatus;
    if ((parseStatus = readPacketField(PKT_F_STRING, tlPacket, &readPos, &result->nickname)) != PKT_PARSE_OK) {
        free(result);
        return parseStatus;
    }
    result->nicknameLen = strlen(result->nickname);

    *packetLogin = result;
    return PKT_PARSE_OK;
}

void tlPackLogin(PacketLogin *packetLogin)
{
    tlPackData(PKT_F_STRING, packetLogin->tlPacket, packetLogin->nickname);
    tlPackHeader(packetLogin->tlPacket);
}

void loginSetNickname(PacketLogin *packetLogin, char *nickname)
{
    uint16_t nicknameLen = strlen(nickname);

    if (packetLogin->nicknameCapacity < nicknameLen + 1) {
        packetLogin->nickname = realloc(packetLogin->nickname, nicknameLen + 1);
        if (packetLogin->nickname == nullptr) {
            DBG_FATAL("Out of memory");
            exit(1);
        }
        packetLogin->nicknameLen = nicknameLen + 1;
        packetLogin->nicknameCapacity = nicknameLen + 1;
        memcpy(packetLogin->nickname, nickname, nicknameLen + 1);
    }
}

void deletePacketLogin(PacketLogin *packetLogin)
{
    // deleteTLPacket(packetLogin->tlPacket);
    if (packetLogin->nicknameCapacity > 0)
        free(packetLogin->nickname);
    free(packetLogin);
}

PacketParseStatus tlUnpackCreateChat(TLPacket *tlPacket, PacketCreateChat **packetCreateChat)
{
    bool newPacket = false;
    PacketCreateChat *result = nullptr;

    if (tlPacket != nullptr && tlPacket->command != CMD_CREATE_CHAT)
        return PKT_PARSE_INVALID_COMMAND;

    if (tlPacket == nullptr) {
        newPacket = true;
        tlPacket = allocTLPacket();
        tlPacket->command = CMD_CREATE_CHAT;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tlPacket = tlPacket;
    result->receiverID = 0;

    if (newPacket) {
        *packetCreateChat = result;
        return PKT_PARSE_OK;
    }

    uint32_t readPos = 0; int parseStatus;
    if ((parseStatus = readPacketField(PKT_F_UINT64, tlPacket, &readPos, &result->receiverID)) != PKT_PARSE_OK) {
        free(result);
        return parseStatus;
    }

    *packetCreateChat = result;
    return PKT_PARSE_OK;
}

void tlPackCreateChat(PacketCreateChat *packetCreateChat)
{
    tlPackData(PKT_F_UINT64, packetCreateChat->tlPacket, &packetCreateChat->receiverID);
    tlPackHeader(packetCreateChat->tlPacket);
}

void createChatSetReceiverID(PacketCreateChat *packetCreateChat, ReceiverID receiverID)
{
    packetCreateChat->receiverID = receiverID;
}

void deletePacketCreateChat(PacketCreateChat *packetCreateChat)
{
    // deleteTLPacket(packetCreateChat->tlPacket);
    free(packetCreateChat);
}

PacketParseStatus tlUnpackServerRespond(TLPacket *tlPacket, PacketServerRespond **packetServerRespond)
{
    bool newPacket = false;
    PacketServerRespond *result = nullptr;

    if (tlPacket != nullptr && tlPacket->command != CMD_SERVER_RESPOND)
        return PKT_PARSE_INVALID_COMMAND;

    if (tlPacket == nullptr) {
        newPacket = true;
        tlPacket = allocTLPacket();
        tlPacket->command = CMD_SERVER_RESPOND;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }
    result->tlPacket = tlPacket;
    result->status = 0;

    if (newPacket) {
        *packetServerRespond = result;
        return PKT_PARSE_OK;
    }

    uint32_t readPos = 0; int parseStatus;
    if ((parseStatus = readPacketField(PKT_F_UINT16, tlPacket, &readPos, &result->status)) != PKT_PARSE_OK) {
        free(result);
        return parseStatus;
    }

    *packetServerRespond = result;
    return PKT_PARSE_OK;
}

void tlPackServerRespond(PacketServerRespond *packetServerRespond)
{
    tlPackData(PKT_F_UINT16, packetServerRespond->tlPacket, &packetServerRespond->status);
    tlPackHeader(packetServerRespond->tlPacket);
}

void serverRespondSetRespond(PacketServerRespond *packetServerRespond, ServerRespond status)
{
    packetServerRespond->status = status;
}

void deletePacketServerRespond(PacketServerRespond *packetServerRespond)
{
    // deleteTLPacket(packetServerRespond->tlPacket);
    free(packetServerRespond);
}