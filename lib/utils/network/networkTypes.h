#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#define PACKET_SIZE_SIZE 4
#define PACKET_TYPE_SIZE 4
#define PACKET_ID_SIZE 4
#define PACKET_HEADER_SIZE (PACKET_SIZE_SIZE + PACKET_TYPE_SIZE + PACKET_ID_SIZE)

#define PACKET_SIZE_OFFSET 0
#define PACKET_TYPE_OFFSET 4
#define PACKET_ID_OFFSET 8
#define PACKET_PAYLOAD_OFFSET 12
#define PACKET_PAYLOAD_NICKNAME_OFFSET 16

#define PACKET_LOGIN_NICKNAME_SZ_OFFSET 12
#define PACKET_LOGIN_NICKNAME_OFFSET 16

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

typedef enum ePacketType
{
    PACKET_LOGIN,
    PACKET_LOGIN_RESPOND,
    PACKET_MESSAGE,
    PACKET_LOGIN_FAILURE,
    PACKET_LOGIN_SUCCESS,
    PACKET_MESSAGE_FAILURE,
    PACKET_MESSAGE_SUCCESS,
    PACKET_CREATE_CHAT,
    PACKET_CREATE_CHAT_FAILURE,
    PACKET_CREATE_CHAT_SUCCESS,
    PACKET_ERROR_CANT_PROCESS,
} PacketType;

#endif //RADIKCHAT_NETWORKTYPES_H
