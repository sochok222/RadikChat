#include "networkTypes.h"
#include <string.h>
#include <stdlib.h>

#define PACKET_BASE_CAPACITY 50

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


}

void addPacketInt(Packet *p, int val)
{
    int size = sizeof(int);

    appendToPacket(p, &size, sizeof(size));
    appendToPacket(p, &val, sizeof(val));
}

void addPacketString(Packet *p, char *string)
{
    uint32_t size = strlen(string) + 1;

    appendToPacket(p, &size, sizeof(size));
    appendToPacket(p, string, size);
}