#include "request.h"
#include "queue.h"
#include "tlPacket.h"
#include <debug.h>

#define PENDING_REQUEST_BUFFER_SIZE 100
#define MAX_PENDING_REQUESTS_BUFFER_SIZE 1024

RequestSlot     requestsSlots[MAX_PENDING_REQUESTS] = { 0 };
static Queue    requestsQueue;
static Queue    oldRequestsQueue;

// TODO add mutex for requestsSlots

void initRequests(void)
{
    initQueue(&requestsQueue);
    for (register uint32_t i = 0; i < MAX_PENDING_REQUESTS; i++) {
        pushToQueue(&requestsQueue, i);
        requestsSlots[i].request = NULL;
        requestsSlots[i].gen = 0;
    }
}

Request *createRequest(void)
{
    if (isQueueEmpty(&requestsQueue)) {
        return NULL; // TODO handle this
    }

    uint16_t requestPos = popFromQueue(&requestsQueue);
    requestsSlots[requestPos].request = malloc(sizeof(Request));
    requestsSlots[requestPos].timeStamp = time(NULL);
    requestsSlots[requestPos].gen++;

    Request *request = requestsSlots[requestPos].request;
    request->id = requestPos;
    request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
    request->mutex = CreateMutex(NULL, FALSE, NULL);
    request->packet = NULL;

    return request;
}

void deleteRequest(Request *request)
{
    // Free request's packet
    free(request->packet);

    // Close handles
    CloseHandle(request->event);
    CloseHandle(request->mutex);

    // Mark request slot as free
    requestsSlots[request->id].request = NULL;
    requestsSlots[request->id].timeStamp = 0;
    pushToQueue(&requestsQueue, request->id);

    // Free memory
    free(request);
}

void associateRequest(TLPacket *packet, Request *request)
{
    tlPacketSetID(packet, request->id);
    tlPacketSetGen(packet, requestsSlots[request->id].gen);
}
