#ifndef RADIKCHAT_REQUESTMANAGER_H
#define RADIKCHAT_REQUESTMANAGER_H

#include <packet/tlPacket.h>
#include <stdint.h>
#include <windows.h>

#define MAX_PENDING_REQUESTS 100
#define MAX_NOTIFICATIONS 100

typedef struct sPendingRequest
{
    int id;

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


Request         *createRequest(void);
void            deleteRequest(Request **request);
void            writeToRequest(Request *request, uint8_t *data, size_t size);

extern RequestSlot  requests[MAX_PENDING_REQUESTS];
extern TLPacket     *notifications[MAX_NOTIFICATIONS];

#endif //RADIKCHAT_REQUESTMANAGER_H
