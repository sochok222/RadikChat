#ifndef RADIKCHAT_NETWORKTYPES_H
#define RADIKCHAT_NETWORKTYPES_H

#define PACKET_TYPE_SIZE 4
#define PACKET_NICKNAME_SIZE 4
#define PACKET_MESSAGE_SIZE 4
#define PACKET_HEADER_SIZE (PACKET_TYPE_SIZE + PACKET_NICKNAME_SIZE)

/* LOGIN structure
 * 0-3bytes - type
 * 4-7bytes - size of nickname
 * 8-*bytes - nickname
 */

/* MESSAGE structure
 * 0-3bytes - type
 * 4-7bytes - size of recipient name
 * 8-*bytes - recipient name
 * ...bytes - message text
 */

typedef enum eMessageType
{
    TYPE_LOGIN,
    TYPE_MESSAGE,
    TYPE_LOGIN_FAILURE,
    TYPE_LOGIN_SUCCESS,
    TYPE_MESSAGE_FAILURE,
    TYPE_MESSAGE_SUCCESS
} MessageType;

#endif //RADIKCHAT_NETWORKTYPES_H
