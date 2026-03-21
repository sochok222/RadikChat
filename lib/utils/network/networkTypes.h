#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#include <stdint.h>
#include <windows.h>

#define NICKNAME_LEN 20
#define MAX_MESSAGE_LEN 250
#define MAX_RETRIES 10

#define PACKET_HEADER_SIZE (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type) + sizeof(((Packet*)(0))->id))
#define PACKET_SIZE_OFFSET (0)
#define PACKET_TYPE_OFFSET (sizeof(((Packet*)(0))->size))
#define PACKET_ID_OFFSET (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type))

typedef enum ePacketType
{
    PACKET_CREATE_CHAT,
    PACKET_CREATE_CHAT_FAILURE,
    PACKET_CREATE_CHAT_SUCCESS,
    PACKET_CREATE_CHAT_CLIENT_NOT_FOUND,
    PACKET_CREATE_CHAT_ITS_YOUR_NICK,

    PACKET_MESSAGE,
    PACKET_MESSAGE_FAILURE,
    PACKET_MESSAGE_CLIENT_NOT_FOUND,
    PACKET_MESSAGE_SUCCESS,

    PACKET_LOGIN_REQUEST,
    PACKET_LOGIN_RESPOND,
    PACKET_LOGIN_FAILURE,
    PACKET_LOGIN_SUCCESS,
    PACKET_LOGIN_ALREADY_EXISTS,

    PACKET_ERROR_CANT_READ,
    PACKET_ERROR_CANT_PROCESS,
    PACKET_ITERNAL_SERVER_ERROR,
} PacketType;

typedef struct sPacket
{
    uint8_t *data;
    PacketType type;
    size_t  size;
    size_t  capacity;
    int id;
} Packet;

typedef enum ePacketFieldType
{
    FIELD_TYPE_INT,
    FIELD_TYPE_UINT,
    FIELD_TYPE_STRING,
} PacketFieldType;

Packet  createPacket(PacketType type, int id);
Packet  packetFromBytes(char *data);
void    deletePacket(Packet packet);
void    sendPacket(SOCKET socket, Packet packet, HANDLE *socketMutex);

// WARNING Caller MUST free memory returned from this functions
int     *readPacketInt(Packet *p, size_t *pos);
char    *readPacketString(Packet *p, size_t *pos);

void appendToPacket(Packet *p, void *buff, size_t len);
void addPacketInt(Packet *p, int val);
void addPacketString(Packet *p, char *string);

/* PACKET structure
 * 0-3bytes     - packetSize
 * 4-7bytes     - packetType
 * 8-11bytes    - packetId
 * 12-*         - payload
 */

/* Login payload
 * 12-15bytes   - nicknameLen
 * 16-*bytes    - nickname
 */

/* Login respond payload
 * 12-15bytes   - fail/success
 */


#endif //RADIKCHAT_NETWORKTYPES_H