#include "packet.h"
#include "debug.h"

PacketParseStatus getPacketLogin(TLPacket *tlPacket, PacketLogin **packetLogin)
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
        DBG_FATAL("Out of memory\n");
        exit(1);
    }
    result->childPacket = tlPacket;
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
    tlPackHeader(packetLogin->childPacket);
    tlPackData(PKT_F_STRING, packetLogin->childPacket, packetLogin->nickname);
}

void loginSetNickname(PacketLogin *packetLogin, char *nickname)
{
    uint16_t nicknameLen = strlen(nickname);

    if (packetLogin->nicknameCapacity < nicknameLen + 1) {
        packetLogin->nickname = realloc(packetLogin->nickname, nicknameLen + 1);
        if (packetLogin->nickname == nullptr) {
            DBG_FATAL("Out of memory\n");
            exit(1);
        }
        packetLogin->nicknameLen = nicknameLen + 1;
        packetLogin->nicknameCapacity = nicknameLen + 1;
    }
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
        tlPacket->command = CMD_LOGIN;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory\n");
        exit(1);
    }
    result->childPacket = tlPacket;
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
    tlPackHeader(packetCreateChat->childPacket);
    tlPackData(PKT_F_UINT64, packetCreateChat->childPacket, &packetCreateChat->receiverID);
}

void createChatSetReceiverID(PacketCreateChat *packetCreateChat, ReceiverID receiverID)
{
    packetCreateChat->receiverID = receiverID;
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
        tlPacket->command = CMD_LOGIN;
    }

    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory\n");
        exit(1);
    }
    result->childPacket = tlPacket;
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
    tlPackHeader(packetServerRespond->childPacket);
    tlPackData(PKT_F_UINT16, packetServerRespond->childPacket, &packetServerRespond->status);
}

void serverRespondSetRespond(PacketServerRespond *packetServerRespond, ServerRespond status)
{
    packetServerRespond->status = status;
}