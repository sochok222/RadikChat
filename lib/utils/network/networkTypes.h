#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#include <stdint.h>

#define PACKET_SIZE_SIZE 4
#define PACKET_TYPE_SIZE 4
#define PACKET_ID_SIZE 4
#define PACKET_HEADER_SIZE (PACKET_SIZE_SIZE + PACKET_TYPE_SIZE + PACKET_ID_SIZE)

#define PACKET_SIZE_OFFSET 0
#define PACKET_TYPE_OFFSET 4
#define PACKET_ID_OFFSET 8
#define PACKET_PAYLOAD_OFFSET 12

#define PACKET_LOGIN_NICKNAME_LEN_OFFSET 12
#define PACKET_LOGIN_NICKNAME_OFFSET 16

#define PACKET_LOGIN_NICKNAME_LEN 4

#define PACKET_MESSAGE_NICKNAME_LEN_OFFSET 12
#define PACKET_MESSAGE_NICKNAME_OFFSET 16

#define PACKET_MESSAGE_NICKNAME_LEN 4
#define PACKET_MESSAGE_TEXT_LEN 4

#define PACKET_CREATE_CHAT_NICKNAME_LEN_OFFSET 12
#define PACKET_CREATE_CHAT_NICKNAME_OFFSET 16

#define PACKET_CREATE_CHAT_NICKNAME_LEN 4

#define NICKNAME_LEN 20
#define MAX_MESSAGE_LEN 250
#define MAX_RETRIES 10

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

    PACKET_LOGIN,
    PACKET_LOGIN_FAILURE,
    PACKET_LOGIN_SUCCESS,
    PACKET_LOGIN_ALREADY_EXISTS,

    PACKET_ERROR_CANT_PROCESS,
    PACKET_ITERNAL_SERVER_ERROR,
} PacketType;

typedef struct sPacket
{
    uint8_t *data;
    PacketType type;
    size_t  size;
    size_t  capacity;
} Packet;

Packet  createPacket(PacketType type);
void    deletePacket(Packet packet);

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
