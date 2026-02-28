//
// Created by sanya on 2/28/2026.
//

#include "NetworkTypes.h"

/* LOGIN structure
 * 0-3bytes - type
 * 4-7bytes - size of nickname
 * 8-*bytes - nickname
 */

/* MESSAGE structure
 * 0-3bytes - type
 * 4-7bytes - size of recipient name
 * 8-*bytes - message text
 */

typedef enum eMessageType
{
    LOGIN,
    MESSAGE
} MessageType;