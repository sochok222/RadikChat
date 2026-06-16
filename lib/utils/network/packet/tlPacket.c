#include "tlPacket.h"

#include "consoleOutput.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

// PCK - packet, PKT_F - packet field
#define PKT_BASE_CAPACITY 150
#define PKT_MAX_CAPACITY 1024
#define PKT_F_MAX_STR_LEN 150
#define PKT_F_STR_LEN_OFFSET (0)
#define PKT_F_STRING_OFFSET (sizeof(size_t))
#define PACKET_CONTENT_OFFSET PACKET_HEADER_SIZE // content offset - begin of payload

static inline size_t getSizeOfType(int fieldType);

TLPacket *allocTLPacket()
{
    TLPacket *packet = malloc(sizeof(*packet));
    if (packet == nullptr) {
        DBG_FATAL("Out of memory\n");
        exit(1);
    }

    packet->capacity  = PKT_BASE_CAPACITY;
    packet->size      = PKT_HEADER_SIZE;
    packet->data      = malloc(PKT_BASE_CAPACITY);

    return packet;
}

PacketParseStatus packetFromBytes(uint8_t *data, TLPacket **packet)
{
    TLPacket *result;
    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory\n");
        exit(1);
    }

    result->id = *(uint64_t*)(data + PKT_ID_OFFSET);

    result->command = *(uint32_t*)(data + PKT_COMMAND_OFFSET);

    result->capacity = 0;
    result->size = *(uint32_t*)(data + PKT_SIZE_OFFSET);
    result->data = NULL;

    if (result->size < PKT_HEADER_SIZE || result->size > PKT_MAX_CAPACITY) {
        *packet = nullptr;
        return PKT_PARSE_SIZE_MISMATCH;
    }

    result->data = malloc(result->size);
    if (result->data == NULL) {
        DBG_FATAL("Out of memory\n");
        exit(1);
    }
    memcpy(result->data + PKT_HEADER_SIZE, data + PKT_HEADER_SIZE, result->size - PKT_HEADER_SIZE);

    *packet = result;
    return PKT_PARSE_OK;
}

void deleteTLPacket(TLPacket *packet)
{
    if (packet->data != NULL)
        free(packet->data);
    free(packet);
}

void sendPacket(SOCKET socket, TLPacket packet, HANDLE *socketMutex)
{
    size_t totalSend = 0;
    uint8_t header[PKT_HEADER_SIZE] = { 0 };
    size_t offset = 0;
    if (socketMutex != NULL)
        WaitForSingleObject(*socketMutex, INFINITE);

    // Packing data to header
    packet.size += PKT_HEADER_SIZE;
    memcpy(header + offset, &packet.size, sizeof(packet.size)); offset += sizeof(packet.size);
    memcpy(header + offset, &packet.command, sizeof(packet.command)); offset += sizeof(packet.command);
    memcpy(header + offset, &packet.id, sizeof(packet.id));

    while (totalSend < PKT_HEADER_SIZE) {
        int sent = send(socket, header + totalSend, PKT_HEADER_SIZE - totalSend, 0);
        if (sent <= 0) {
            DBG_FATAL("Can`t send packet to the server\n");
            printNotification(formatError, "can`t send packet to the server");
            exit(1);
        }
        totalSend += sent;
    }

    packet.size -= PKT_HEADER_SIZE;

    totalSend = 0;
    while (totalSend < packet.size) {
        int maxSend = packet.size - totalSend > INT_MAX ? INT_MAX : (int)(packet.size - totalSend);
        int sent = send(socket, (char*)packet.data + totalSend, maxSend, 0);
        if (sent <= 0) {
            DBG_FATAL("Can`t send packet to the server\n");
            printNotification(formatError, "can`t send packet to the server");
            exit(1);
        }
        totalSend += sent;
    }

    if (socketMutex != NULL)
        ReleaseMutex(*socketMutex);
}

void tlPackHeader(TLPacket *packet)
{
    size_t offset = 0;

    // Packing header to packet's buffer
    memcpy(packet->data + offset, &packet->size, sizeof(packet->size)); offset += sizeof(packet->size);
    memcpy(packet->data + offset, &packet->command, sizeof(packet->command)); offset += sizeof(packet->command);
    memcpy(packet->data + offset, &packet->id, sizeof(packet->id));
}

void tlPackData(int fieldType, TLPacket *packet, const void *data)
{
    size_t fieldSize;
    if (fieldType == PKT_F_STRING)
        fieldSize = strlen((char*)data);
    else
        fieldSize = getSizeOfType(fieldType);

    if (packet->size + fieldSize > packet->capacity) {
        packet->data = realloc(packet->data, packet->size + fieldSize);
        packet->capacity = packet->size + fieldSize;
    }
    memcpy(packet->data + packet->size, data, fieldSize);
    packet->size += fieldSize;
}

static PacketParseStatus readPacketField(int fieldType, TLPacket *packet, uint32_t *readPos, void *out)
{
    size_t fieldSize;
    if (*readPos == 0) {
        *readPos = PKT_HEADER_SIZE; // payload starts after header
    }
    if (packet->size - *readPos < (fieldSize = getSizeOfType(fieldType)))
       return PKT_PARSE_SIZE_MISMATCH;

    switch (fieldType) {
    case PKT_F_STRING:
        fieldSize = *(size_t*)(packet->data + *readPos + PKT_F_STR_LEN_OFFSET);
        if (packet->size - *readPos < fieldSize)
            return PKT_PARSE_SIZE_MISMATCH;
        *((char**)out) = (char*)(packet->data + *readPos + PKT_F_STRING_OFFSET);
        break;
    case PKT_F_UINT8:
        *((uint8_t*)out) = *(uint8_t*)(packet->data);
        break;
    case PKT_F_UINT16:
        *((uint16_t*)out) = *(uint16_t*)(packet->data);
        break;
    case PKT_F_UINT32:
        *((uint32_t*)out) = *(uint32_t*)(packet->data);
        break;
    case PKT_F_UINT64:
        *((uint64_t*)out) = *(uint64_t*)(packet->data);
        break;
    default:
        return PKT_PARSE_UNKNOWN_FIELD_TYPE;
    }

    *readPos += fieldSize;
}

static size_t getSizeOfType(int fieldType)
{
    switch (fieldType) {
    case PKT_F_UINT8:  return sizeof(uint8_t);
    case PKT_F_UINT16: return sizeof(uint16_t);
    case PKT_F_UINT32: return sizeof(uint32_t);
    case PKT_F_UINT64: return sizeof(uint64_t);
    default: return 1;
    }
}

const char *getParseStatusString(PacketParseStatus parseStatus)
{
    switch (parseStatus) {
    case PKT_PARSE_OK:
        return "ok";
    case PKT_PARSE_SIZE_MISMATCH:
        return "size mismatch";
    case PKT_PARSE_STR_NOT_TERMINATED:
        return "not terminated string";
    case PKT_PARSE_UNKNOWN_FIELD_TYPE:
        return "unknown field type";
    case PKT_PARSE_INVALID_COMMAND:
        return "command not valid";
    default:
        return "unknown parse status";
    }
}
