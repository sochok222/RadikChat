#include "packet.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>


#define PACKET_BASE_CAPACITY 50
#define PACKET_MAX_CAPACITY 500
#define PACKET_FIELD_MAX_STRING_LEN 150
#define PACKET_CONTENT_OFFSET PACKET_HEADER_SIZE

#define PACKET_FIELD_HEADER_SIZE (sizeof(int) + sizeof(size_t))
#define PACKET_FIELD_TYPE_OFFSET (0)
#define PACKET_FIELD_SIZE_OFFSET (sizeof(int))
#define PACKET_FIELD_CONTENT_OFFSET (sizeof(int) + sizeof(size_t))

Packet createPacket(PacketType type, PacketCommand command, PacketStatus status, int id)
{
    Packet p;

    p.id = id;

    p.type      = type;
    p.command   = command;
    p.status    = status;

    p.capacity  = PACKET_BASE_CAPACITY;
    p.size      = 0;
    p.data      = malloc(PACKET_BASE_CAPACITY);

    return p;
}

Packet packetFromBytes(uint8_t *data)
{
    Packet p;

    p.id = *(int*)(data + PACKET_ID_OFFSET);

    p.type = *(int*)(data + PACKET_TYPE_OFFSET);
    p.command = *(int*)(data + PACKET_COMMAND_OFFSET);
    p.status = *(int*)(data + PACKET_STATUS_OFFSET);

    p.capacity = 0;
    p.size = *(size_t*)(data + PACKET_SIZE_OFFSET);
    p.data = NULL;

    p.parseError = PARSE_ERROR_NONE;

    if (p.size - PACKET_HEADER_SIZE <= PACKET_MAX_CAPACITY) {
        if (p.size - PACKET_HEADER_SIZE > 0)
            p.data = malloc(p.size);
        else {
            p.parseError = PARSE_ERROR_TOO_SMALL;
            return p;
        }
        if (p.data == NULL) {
            DBG_ERROR("Cant allocate memory for packet data\n");
            p.parseError = PARSE_ERROR_MALLOC_FAILED;
        } else
            memcpy(p.data, data + PACKET_HEADER_SIZE, p.size - PACKET_HEADER_SIZE);
    } else
        p.parseError = PARSE_ERROR_TOO_BIG;

    return p;
}

void deletePacket(Packet packet)
{
    if (packet.data != NULL)
        free(packet.data);
    packet.data = NULL;
}

void sendPacket(SOCKET socket, Packet packet, HANDLE *socketMutex)
{
    int toSend = 0;
    if (socketMutex != NULL)
        WaitForSingleObject(*socketMutex, INFINITE);

    packet.size += PACKET_HEADER_SIZE;
    send(socket, (char*)&packet.size, sizeof(packet.size), 0);
    send(socket, (char*)&packet.type, sizeof(packet.type), 0);
    send(socket, (char*)&packet.command, sizeof(packet.command), 0);
    send(socket, (char*)&packet.status, sizeof(packet.status), 0);
    send(socket, (char*)&packet.id, sizeof(packet.id), 0);
    packet.size -= PACKET_HEADER_SIZE;
    while (packet.size) {
        toSend = packet.size > INT_MAX ? INT_MAX : (int)packet.size;
        if (toSend == INT_MAX)
            packet.size -= INT_MAX;
        else
            packet.size -= toSend;
        send(socket, (char*)packet.data, toSend, 0);
    }

    if (socketMutex != NULL)
        ReleaseMutex(*socketMutex);
}

void appendToPacket(Packet *p, const void *buff, size_t len)
{
    const char *buffer = (char*)buff;
    if (p->size + len > p->capacity) {
        p->data = realloc(p->data, p->size + len);
        p->capacity = p->size + len;
    }
    memcpy(p->data + p->size, buffer, len);
    p->size += len;
}

void addPacketInt(Packet *p, const int val)
{
    int type = FIELD_TYPE_INT;
    size_t size = sizeof(int);

    appendToPacket(p, &type, sizeof(type));
    appendToPacket(p, &size, sizeof(size));
    appendToPacket(p, &val, sizeof(val));
}

void addPacketString(Packet *p, const char *string)
{
    int type = FIELD_TYPE_STRING;
    size_t size = strlen(string) + 1;

    appendToPacket(p, &type, sizeof(type));
    appendToPacket(p, &size, sizeof(size));
    appendToPacket(p, string, size);
}

char *readPacketString(Packet *p, size_t *pos)
{
    size_t typeSize;
    char *result;

    if (p->size < *pos + PACKET_FIELD_HEADER_SIZE) {
        DBG_ERROR("Field does not contain enough space for size\n");
        return NULL;
    }

    if (*(int*)(p->data + *pos + PACKET_FIELD_TYPE_OFFSET) != FIELD_TYPE_STRING) {
        DBG_ERROR("Not correct field type\n");
        return NULL;
    }

    typeSize = *(size_t*)(p->data + *pos + PACKET_FIELD_SIZE_OFFSET);
    if (typeSize > p->size - *pos || typeSize > PACKET_FIELD_MAX_STRING_LEN) {
        DBG_ERROR("Not correct field size\n");
        return NULL;
    }

    result = (char*)(p->data + *pos + PACKET_FIELD_CONTENT_OFFSET);

    if (*(result + typeSize - 1) != '\0') {
        DBG_ERROR("String field in not null-terminated\n")
        return NULL;
    }

    return result;
}

int *readPacketInt(Packet *p, size_t *pos)
{
    size_t typeSize;
    int *result;
    // Check if can read header
    if (p->size < *pos + PACKET_FIELD_HEADER_SIZE) {
        DBG_ERROR("Field does not contain enough space for size\n")
        return NULL;
    }

    if (*(int*)(p->data + *pos + PACKET_FIELD_TYPE_OFFSET) != FIELD_TYPE_INT) {
        DBG_ERROR("Not correct field type\n");
        return NULL;
    }

    typeSize = *(size_t*)(p->data + *pos + PACKET_FIELD_SIZE_OFFSET);
    if (typeSize > p->size - *pos || typeSize > sizeof(int)) {
        DBG_ERROR("Not correct field size\n");
        return NULL;
    }

    result = (int*)(p->data + *pos + PACKET_FIELD_CONTENT_OFFSET);

    *pos += PACKET_FIELD_HEADER_SIZE + typeSize;

    return result;
}