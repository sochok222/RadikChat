#include "networkTypes.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>

#define TYPE_PACKET_HEADER_SIZE 8 // field type + size
#define TYPE_PACKET_TYPE_OFFSET (0)
#define TYPE_PACKET_SIZE_OFFSET (sizeof(int))
#define TYPE_PACKET_CONTENT_OFFSET (sizeof(int) + sizeof(size_t))

#define PACKET_BASE_CAPACITY 50
#define TYPE_PACKET_MAX_STRING_LEN 150

Packet createPacket(PacketType type)
{
    Packet p;

    p.type = type;
    p.capacity = PACKET_BASE_CAPACITY;
    p.size = 0;
    p.data = malloc(PACKET_BASE_CAPACITY);

    return p;
}

void deletePacket(Packet packet)
{
    free(packet.data);
}

void appendToPacket(Packet *p, void *buff, size_t len)
{
    char *buffer = (char*)buff;
    if (p->size + len > p->capacity) {
        p->data = realloc(p->data, p->size + len);
        p->capacity = p->size + len;
    }
    memcpy(p->data + p->size, buffer, len);
    p->size += len;
}

void addPacketInt(Packet *p, int val)
{
    int type = FIELD_TYPE_INT;
    size_t size = sizeof(int);

    appendToPacket(p, &type, sizeof(type));
    appendToPacket(p, &size, sizeof(size));
    appendToPacket(p, &val, sizeof(val));
}

void addPacketString(Packet *p, char *string)
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

    if (p->size < *pos + TYPE_PACKET_HEADER_SIZE) {
        DBG_ERROR("Field does not contain enough space for size\n");
        return NULL;
    }

    if (*(int*)(p->data + *pos + TYPE_PACKET_TYPE_OFFSET) != FIELD_TYPE_STRING) {
        DBG_ERROR("Not correct field type\n");
        return NULL;
    }

    typeSize = *(size_t*)(p->data + *pos + TYPE_PACKET_SIZE_OFFSET);
    if (typeSize > p->size - *pos || typeSize > TYPE_PACKET_MAX_STRING_LEN) {
        DBG_ERROR("Not correct field size\n");
        return NULL;
    }

    result = malloc(sizeof(char) * typeSize);
    memcpy(result, p->data + *pos + TYPE_PACKET_CONTENT_OFFSET, typeSize);

    if (*(result + typeSize - 1) != '\0') {
        DBG_WARNING("String field in not null-terminated\n")
    }

    return result;
}

int *readPacketInt(Packet *p, size_t *pos)
{
    size_t typeSize;
    int *result;
    // Check if can read header
    if (p->size < *pos + TYPE_PACKET_HEADER_SIZE) {
        DBG_ERROR("Field does not contain enough space for size\n")
        return NULL;
    }

    if (*(int*)(p->data + *pos + TYPE_PACKET_TYPE_OFFSET) != FIELD_TYPE_INT) {
        DBG_ERROR("Not correct field type\n");
        return NULL;
    }

    typeSize = *(size_t*)(p->data + *pos + TYPE_PACKET_SIZE_OFFSET);
    if (typeSize > p->size - *pos || typeSize > sizeof(int)) {
        DBG_ERROR("Not correct field size\n");
        return NULL;
    }

    result = malloc(sizeof(int)); // TODO add malloc error handling
    *result = *(int*)(p->data + *pos + TYPE_PACKET_CONTENT_OFFSET);

    *pos += TYPE_PACKET_HEADER_SIZE + typeSize;

    return result;
}