#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#include <stdint.h>
#include <windows.h>

#define NICKNAME_LEN 20
#define MAX_MESSAGE_LEN 250
#define MAX_RETRIES 10

#define MAX_PACKET_SIZE ((uint32_t)(1024 * 2)) // 2KB per packet

#define PKT_SIZE_OFFSET (0)

#define PKT_COMMAND_OFFSET (sizeof(((TlPacket*)(0))->size))

#define PKT_ID_OFFSET (sizeof(((TlPacket*)(0))->size) + \
                          sizeof(((TlPacket*)(0))->command))

#define PKT_HEADER_SIZE (sizeof(((TlPacket*)(0))->size) + \
                            sizeof(((TlPacket*)(0))->command) + \
                            sizeof(((TlPacket*)(0))->id))

enum PacketCommand
{
    CMD_CREATE_CHAT = 0,
    CMD_MESSAGE,
    CMD_LOGIN,
    CMD_SERVER_RESPOND,
};

enum ServerRespond
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

typedef enum PacketParseStatus
{
    PKT_PARSE_OK = 0,
    PKT_PARSE_SIZE_MISMATCH, // expected payload size != actual
    PKT_PARSE_STR_NOT_TERMINATED,
    PKT_PARSE_UNKNOWN_FIELD_TYPE,
    PKT_PARSE_INVALID_COMMAND,
} PacketParseStatus;

static enum PacketFieldType
{
    PKT_F_STRING = 0,
    PKT_F_UINT8,
    PKT_F_UINT16,
    PKT_F_UINT32,
    PKT_F_UINT64,
} PacketFieldType;

// TODO
// Add magic fields(0xFF00FFFF, 0xCAFEBABE etc.)
// to packet begin/end to indicate bounds and
// make packet receive stream restoration possible

typedef uint32_t TlPacketSize;
typedef uint32_t TlPacketCommand;
typedef uint64_t TlPacketId;        // 32 lower bits - actual id, 32 higher bits - generation
typedef uint32_t TlPacketCapacity;  // Not used in protocol

typedef struct TlPacket // Tl - transport layer
{
    TlPacketSize        size;
    TlPacketCommand     command;
    TlPacketId          id;
    TlPacketCapacity    capacity;
    uint8_t             *data;
} TlPacket;

const char *get_parse_status_string(PacketParseStatus parse_status);

[[nodiscard]] TlPacket          *alloc_tl_packet();
[[nodiscard]] PacketParseStatus packet_from_bytes(uint8_t *data, TlPacket **packet);

PacketParseStatus read_packet_field(int field_type, TlPacket *packet, uint32_t *read_pos, void *out);
void tl_pack_data(int field_type, TlPacket *packet, const void *data);

void delete_tl_packet(TlPacket *packet);
void send_packet(SOCKET socket, TlPacket packet, HANDLE *socket_mutex);
void tl_pack_header(TlPacket *packet);

inline uint16_t tl_packet_get_id(TlPacket *packet);
inline uint16_t tl_packet_get_gen(TlPacket *packet);
inline void     tl_packet_set_id(TlPacket *packet, uint16_t id);
inline void     tl_packet_set_gen(TlPacket *packet, uint16_t gen);

#endif //RADIKCHAT_NETWORKTYPES_H