#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#include <stdint.h>
#include <windows.h>

#define NICKNAME_LEN 20
#define MAX_MESSAGE_LEN 250
#define MAX_RETRIES 10

#define PACKET_HEADER_SIZE (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type) + \
                            sizeof(((Packet*)(0))->command) + sizeof(((Packet*)(0))->status)) + \
                            sizeof(((Packet*)(0))->id)

#define PACKET_SIZE_OFFSET (0)
#define PACKET_TYPE_OFFSET (sizeof(((Packet*)(0))->size))
#define PACKET_COMMAND_OFFSET (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type))

#define PACKET_STATUS_OFFSET (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type) + \
                              sizeof(((Packet*)(0))->command))

#define PACKET_ID_OFFSET (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type) + \
                          sizeof(((Packet*)(0))->command) + sizeof(((Packet*)(0))->status))

typedef enum ePacketType
{
    TYPE_REQUEST,
    TYPE_RESPOND,
    TYPE_DELIVERY,
} PacketType;

typedef enum ePacketCommand
{
    COMMAND_CREATE_CHAT,
    COMMAND_MESSAGE,
    COMMAND_LOGIN,
    COMMAND_NONE
} PacketCommand;

typedef enum eStatus
{
    STATUS_OK,
    STATUS_FAILURE,
    STATUS_CANT_READ,
    STATUS_NOT_FOUND,
    STATUS_ALREADY_EXISTS,
    STATUS_SERVER_ERROR,
    STATUS_SIZE_TOO_BIG,
    STATUS_ACTION_TO_HIMSELF, // Client tries to create chat/send message to himself
} PacketStatus;

typedef enum ePacketParseError
{
    PARSE_ERROR_NONE,
    PARSE_ERROR_TOO_BIG,
    PARSE_ERROR_MALLOC_FAILED ,
    PARSE_ERROR_TOO_SMALL,
} PacketParseError;

typedef struct sPacket
{
    int id;

    PacketType      type;
    PacketCommand   command;
    PacketStatus    status;

    size_t  capacity;
    size_t  size;
    uint8_t *data;

    PacketParseError parseError;
} Packet;

typedef enum ePacketFieldType
{
    FIELD_TYPE_INT,
    FIELD_TYPE_UINT,
    FIELD_TYPE_STRING,
} PacketFieldType;

Packet  createPacket(PacketType type, PacketCommand command, PacketStatus status, int id);
Packet  packetFromBytes(uint8_t *data);
void    deletePacket(Packet packet);
void    sendPacket(SOCKET socket, Packet packet, HANDLE *socketMutex);

// WARNING do not free memory returned from this functions
int     *readPacketInt(Packet *p, size_t *pos);
char    *readPacketString(Packet *p, size_t *pos);
//

void appendToPacket(Packet *p, const void *buff, size_t len);
void addPacketInt(Packet *p, const int val);
void addPacketString(Packet *p, const char *string);

#endif //RADIKCHAT_NETWORKTYPES_H