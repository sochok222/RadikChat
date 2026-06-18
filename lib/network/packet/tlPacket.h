#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#include <stdint.h>
#include <windows.h>

#define NICKNAME_LEN 20
#define MAX_MESSAGE_LEN 250
#define MAX_RETRIES 10

#define PKT_SIZE_OFFSET (0)

#define PKT_COMMAND_OFFSET (sizeof(((TLPacket*)(0))->size))

#define PKT_ID_OFFSET (sizeof(((TLPacket*)(0))->size) + \
                          sizeof(((TLPacket*)(0))->command))

#define PKT_HEADER_SIZE (sizeof(((TLPacket*)(0))->size) + \
                            sizeof(((TLPacket*)(0))->command) + \
                            sizeof(((TLPacket*)(0))->id))

enum ePacketCommand
{
    CMD_CREATE_CHAT = 0,
    CMD_MESSAGE,
    CMD_LOGIN,
    CMD_SERVER_RESPOND,
};
typedef uint32_t PacketCommand;

enum eServerRespond
{
    SERV_RESPOND_OK = 0,
    SERV_RESPOND_FAILURE,
    SERV_RESPOND_CANT_PARSE,
    SERV_RESPOND_NOT_FOUND,
    SERV_RESPOND_ALREADY_EXISTS,
    SERV_RESPOND_SERVER_ERROR,
    SERV_RESPOND_SIZE_MISMATCH,
    SERV_RESPOND_ACTION_TO_HIMSELF, // Client tries to create chat or send message to himself
    SERV_RESPOND_NICKNAME_TOO_SHORT,
    SERV_RESPOND_NICKNAME_TOO_LONG,
};
typedef uint16_t ServerRespond;

typedef enum ePacketParseStatus
{
    PKT_PARSE_OK = 0,
    PKT_PARSE_SIZE_MISMATCH, // expected payload size != actual
    PKT_PARSE_STR_NOT_TERMINATED,
    PKT_PARSE_UNKNOWN_FIELD_TYPE,
    PKT_PARSE_INVALID_COMMAND,
} PacketParseStatus;

static enum ePacketFieldType
{
    PKT_F_STRING = 0,
    PKT_F_UINT8,
    PKT_F_UINT16,
    PKT_F_UINT32,
    PKT_F_UINT64,
} PacketFieldType;

// 32 lower bits - actual id, 32 higher bits - generation
typedef uint64_t PacketID;

// TODO
// Add magic fields(0xFF00FFFF, 0xCAFEBABE etc.)
// to packet begin/end to indicate bounds and
// make packet receive stream restoration possible

typedef struct sTLPacket // TL - transport layer
{
    uint32_t        size;
    PacketCommand   command;
    PacketID        id;
    uint32_t        capacity;
    uint8_t         *data;
} TLPacket;

const char *getParseStatusString(PacketParseStatus parseStatus);

[[nodiscard]] TLPacket          *allocTLPacket();
[[nodiscard]] PacketParseStatus packetFromBytes(uint8_t *data, TLPacket **packet);

PacketParseStatus readPacketField(int fieldType, TLPacket *packet, uint32_t *readPos, void *out);
void tlPackData(int fieldType, TLPacket *packet, const void *data);

void deleteTLPacket(TLPacket *packet);
void sendPacket(SOCKET socket, TLPacket packet, HANDLE *socketMutex);
void tlPackHeader(TLPacket *packet);


#endif //RADIKCHAT_NETWORKTYPES_H