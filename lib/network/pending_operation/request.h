#ifndef RADIKCHAT_REQUESTMANAGER_H
#define RADIKCHAT_REQUESTMANAGER_H

#include <stdint.h>
#include <tl_packet.h>
#include <windows.h>

#define MAX_PENDING_REQUESTS 100

typedef struct sPendingRequest
{
    uint16_t id;

    HANDLE mutex;
    HANDLE event;

    TLPacket *packet;
} Request;

typedef struct sRequestSlot
{
    Request *request;
    uint64_t time_stamp; // epoch time when request was created
    uint16_t gen;
} RequestSlot;

void            init_requests(void);
Request         *create_request(void);
void            delete_request(Request *request);
void            associate_request(TLPacket *packet, Request *request);

extern RequestSlot  g_requests_slots[MAX_PENDING_REQUESTS];

#endif //RADIKCHAT_REQUESTMANAGER_H
