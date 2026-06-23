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

static inline size_t get_size_of_type(int file_type);

TLPacket *alloc_tl_packet()
{
    TLPacket *packet = malloc(sizeof(*packet));
    if (packet == nullptr) {
        DBG_FATAL("Out of memory");
        exit(1);
    }

    packet->capacity  = PKT_BASE_CAPACITY;
    packet->size      = PKT_HEADER_SIZE;
    packet->data      = malloc(PKT_BASE_CAPACITY);

    return packet;
}

PacketParseStatus packet_from_bytes(uint8_t *data, TLPacket **packet)
{
    TLPacket *result;
    if ((result = malloc(sizeof(*result))) == nullptr) {
        DBG_FATAL("Out of memory");
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
        DBG_FATAL("Out of memory");
        exit(1);
    }
    memcpy(result->data, data, result->size);

    *packet = result;
    return PKT_PARSE_OK;
}

void delete_tl_packet(TLPacket *packet)
{
    if (packet->data != NULL)
        free(packet->data);
    free(packet);
}

void send_packet(SOCKET socket, TLPacket packet, HANDLE *socket_mutex)
{
    size_t total_send = 0;
    uint8_t header[PKT_HEADER_SIZE] = { 0 };
    size_t offset = 0;
    if (socket_mutex != NULL)
        WaitForSingleObject(*socket_mutex, INFINITE);

    // Packing data to header
    packet.size += PKT_HEADER_SIZE;
    memcpy(header + offset, &packet.size, sizeof(packet.size)); offset += sizeof(packet.size);
    memcpy(header + offset, &packet.command, sizeof(packet.command)); offset += sizeof(packet.command);
    memcpy(header + offset, &packet.id, sizeof(packet.id));

    while (total_send < PKT_HEADER_SIZE) {
        int sent = send(socket, header + total_send, PKT_HEADER_SIZE - total_send, 0);
        if (sent <= 0) {
            DBG_FATAL("Can`t send packet to the server");
            print_notification(formatError, "can`t send packet to the server");
            exit(1);
        }
        total_send += sent;
    }

    packet.size -= PKT_HEADER_SIZE;

    total_send = 0;
    while (total_send < packet.size) {
        int max_send = packet.size - total_send > INT_MAX ? INT_MAX : (int)(packet.size - total_send);
        int sent = send(socket, (char*)packet.data + total_send, max_send, 0);
        if (sent <= 0) {
            DBG_FATAL("Can`t send packet to the server\n");
            print_notification(formatError, "can`t send packet to the server");
            exit(1);
        }
        total_send += sent;
    }

    if (socket_mutex != NULL)
        ReleaseMutex(*socket_mutex);
}

void tl_pack_header(TLPacket *packet)
{
    size_t offset = 0;

    // Packing header to packet's buffer
    memcpy(packet->data + offset, &packet->size, sizeof(packet->size)); offset += sizeof(packet->size);
    memcpy(packet->data + offset, &packet->command, sizeof(packet->command)); offset += sizeof(packet->command);
    memcpy(packet->data + offset, &packet->id, sizeof(packet->id));
}

void tl_pack_data(int field_type, TLPacket *packet, const void *data)
{
    size_t field_size;
    if (field_type == PKT_F_STRING)
        field_size = strlen((char*)data);
    else
        field_size = get_size_of_type(field_type);

    if (packet->size + field_size > packet->capacity) {
        packet->data = realloc(packet->data, packet->size + field_size + 10); // NOTE + 10 is temporary solution
        packet->capacity = packet->size + field_size;
    }
    if (field_type == PKT_F_STRING) {
        field_size++; // Need to send null symbol
        memcpy(packet->data + packet->size, &field_size, sizeof(field_size));
        packet->size += sizeof(field_size);
        memcpy(packet->data + packet->size, data, field_size + 1);
    } else {
        memcpy(packet->data + packet->size, data, field_size);
    }
    packet->size += field_size;
}

PacketParseStatus read_packet_field(int field_type, TLPacket *packet, uint32_t *read_pos, void *out)
{
    size_t field_size;
    if (*read_pos == 0) {
        *read_pos = PKT_HEADER_SIZE; // payload starts after header
    }
    if (packet->size - *read_pos < (field_size = get_size_of_type(field_type)))
       return PKT_PARSE_SIZE_MISMATCH;

    switch (field_type) {
    case PKT_F_STRING:
        field_size = *(size_t*)(packet->data + *read_pos + PKT_F_STR_LEN_OFFSET);
        if (packet->size - *read_pos < field_size)
            return PKT_PARSE_SIZE_MISMATCH;
        *((char**)out) = (char*)(packet->data + *read_pos + PKT_F_STRING_OFFSET);
        break;
    case PKT_F_UINT8:
        *((uint8_t*)out) = *(uint8_t*)(packet->data + *read_pos);
        break;
    case PKT_F_UINT16:
        *((uint16_t*)out) = *(uint16_t*)(packet->data + *read_pos);
        break;
    case PKT_F_UINT32:
        *((uint32_t*)out) = *(uint32_t*)(packet->data + *read_pos);
        break;
    case PKT_F_UINT64:
        *((uint64_t*)out) = *(uint64_t*)(packet->data + *read_pos);
        break;
    default:
        return PKT_PARSE_UNKNOWN_FIELD_TYPE;
    }

    *read_pos += field_size;
    return PKT_PARSE_OK;
}

static size_t get_size_of_type(int file_type)
{
    switch (file_type) {
    case PKT_F_UINT8:  return sizeof(uint8_t);
    case PKT_F_UINT16: return sizeof(uint16_t);
    case PKT_F_UINT32: return sizeof(uint32_t);
    case PKT_F_UINT64: return sizeof(uint64_t);
    default: return 1;
    }
}

const char *get_parse_status_string(PacketParseStatus parse_status)
{
    switch (parse_status) {
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

uint16_t tl_packet_get_id(TLPacket *packet)
{
    return packet->id & 0xFFFF;
}

uint16_t tl_packet_get_gen(TLPacket *packet)
{
    return packet->id >> 16;
}

void tl_packet_set_id(TLPacket *packet, uint16_t id)
{
    packet->id &= ~0xFFFF;
    packet->id |= id;
}

void tl_packet_set_gen(TLPacket *packet, uint16_t gen)
{
    packet->id &= ~0xFFFF0000;
    packet->id |= (gen << 16);
}
