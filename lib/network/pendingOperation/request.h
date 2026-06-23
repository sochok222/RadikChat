#ifndef RADIKCHAT_REQUESTMANAGER_H
#define RADIKCHAT_REQUESTMANAGER_H

#include <tlPacket.h>
#include <stdint.h>
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
    uint64_t timeStamp; // epoch time when request was created
    uint16_t gen;
} RequestSlot;

void            initRequests(void);
Request         *createRequest(void);
void            deleteRequest(Request *request);
void            associateRequest(TLPacket *packet, Request *request);

extern RequestSlot  requestsSlots[MAX_PENDING_REQUESTS];

#endif //RADIKCHAT_REQUESTMANAGER_H
