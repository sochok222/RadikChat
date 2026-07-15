#include "request.h"
#include "../../miscellaneous/miscellaneous.h"
#include "queue.h"
#include "tl_packet.h"

#include <debug.h>

#define PENDING_REQUEST_BUFFER_SIZE 100
#define MAX_PENDING_REQUESTS_BUFFER_SIZE 1024

RequestSlot     g_requests_slots[MAX_PENDING_REQUESTS] = { 0 };
static Queue    requests_queue;
static Queue    old_requests_queue;

// TODO add mutex for g_requests_slots

void init_requests(void)
{
    init_queue(&requests_queue);
    init_queue(&old_requests_queue);
    for (register uint32_t i = 0; i < MAX_PENDING_REQUESTS; i++) {
        push_to_queue(&requests_queue, i);
        g_requests_slots[i].request = NULL;
        g_requests_slots[i].gen = 0;
    }
}

Request *create_request(void)
{
    if (is_queue_empty(&requests_queue)) {
        return NULL; // TODO handle this
    }

    uint16_t requestPos = pop_from_queue(&requests_queue);
    push_to_queue(&old_requests_queue, requestPos);
    g_requests_slots[requestPos].request = malloc(sizeof(Request));
    g_requests_slots[requestPos].time_stamp = time(NULL);
    g_requests_slots[requestPos].gen++;

    Request *request = g_requests_slots[requestPos].request;
    request->id = requestPos;
    request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
    request->mutex = CreateMutex(NULL, FALSE, NULL);
    request->packet = NULL;

    return request;
}

void delete_request(Request *request)
{
    // Free request's packet
    free(request->packet);

    // Close handles
    safe_close_handle(request->event);
    safe_close_handle(request->mutex);

    // Mark request slot as free
    g_requests_slots[request->id].request = NULL;
    g_requests_slots[request->id].time_stamp = 0;
    push_to_queue(&requests_queue, request->id);
    pop_from_queue(&old_requests_queue);

    // Free memory
    free(request);
}

void associate_request(TLPacket *packet, Request *request)
{
    tl_packet_set_id(packet, request->id);
    tl_packet_set_gen(packet, g_requests_slots[request->id].gen);
}
