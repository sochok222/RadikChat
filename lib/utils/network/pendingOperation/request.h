#ifndef RADIKCHAT_REQUESTMANAGER_H
#define RADIKCHAT_REQUESTMANAGER_H

#include <packetManager/packet.h>
#include <stdint.h>
#include <windows.h>

#define MAX_PENDING_REQUESTS 100
#define MAX_NOTIFICATIONS 100

typedef struct sPendingRequest
{
    int id;

    HANDLE mutex;
    HANDLE event;

    size_t  size;
    size_t  capacity;
    uint8_t *data;
} PendingRequest;


PendingRequest  *createRequest(void);
void            deleteRequest(PendingRequest *request);
void            writeToRequest(PendingRequest *request, uint8_t *data, size_t size);

extern PendingRequest   *pendingRequests[MAX_PENDING_REQUESTS];
extern Packet           *notifications[MAX_NOTIFICATIONS];

#endif //RADIKCHAT_REQUESTMANAGER_H
