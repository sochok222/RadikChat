#include "networkTypes.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>


#define PACKET_BASE_CAPACITY 50
#define PACKET_MAX_CAPACITY 500
#define PACKET_FIELD_MAX_STRING_LEN 150
#define PACKET_CONTENT_OFFSET (sizeof(((Packet*)(0))->size) + sizeof(((Packet*)(0))->type) + sizeof(((Packet*)(0))->id))

#define PACKET_FIELD_HEADER_SIZE (sizeof(int) + sizeof(size_t))
#define PACKET_FIELD_TYPE_OFFSET (0)
#define PACKET_FIELD_SIZE_OFFSET (sizeof(int))
#define PACKET_FIELD_CONTENT_OFFSET (sizeof(int) + sizeof(size_t))

Packet createPacket(PacketType type, int id)
{
    Packet p;

    p.type = type;
    p.capacity = PACKET_BASE_CAPACITY;
    p.size = 0;
    p.data = malloc(PACKET_BASE_CAPACITY);
    p.id = id;

    return p;
}

Packet packetFromBytes(char *request)
{
    Packet p;

    p.type = *(int*)(request + PACKET_TYPE_OFFSET);
    p.capacity = 0;
    p.size = *(size_t*)(request + PACKET_SIZE_OFFSET);
    p.id = *(int*)(request + PACKET_ID_OFFSET);
    p.data = NULL;

    if (p.size <= PACKET_MAX_CAPACITY) {
        p.data = malloc(p.size);
        if (p.data == NULL) {
            DBG_ERROR("Cant allocate memory for packet data\n");
        } else
            memcpy(p.data, request + PACKET_HEADER_SIZE, p.size - PACKET_HEADER_SIZE);
    }

    return p;
}

void deletePacket(Packet packet)
{
    free(packet.data);
}

void sendPacket(SOCKET socket, Packet packet, HANDLE *socketMutex)
{
    if (socketMutex != NULL)
        WaitForSingleObject(*socketMutex, INFINITE);

    packet.size = PACKET_HEADER_SIZE + packet.size;
    send(socket, (char*)&packet.size, sizeof(packet.size), 0);
    send(socket, (char*)&packet.type, sizeof(packet.type), 0);
    send(socket, (char*)&packet.id, sizeof(packet.id), 0);
    send(socket, (char*)packet.data, packet.size - PACKET_HEADER_SIZE, 0);

    if (socketMutex != NULL)
        ReleaseMutex(*socketMutex);
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