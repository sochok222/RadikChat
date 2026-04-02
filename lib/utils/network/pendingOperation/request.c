#include "request.h"
#include <debug.h>

#define PENDING_REQUEST_BUFFER_SIZE 100
#define MAX_PENDING_REQUESTS_BUFFER_SIZE 1024

PendingRequest  *pendingRequests[MAX_PENDING_REQUESTS] = { 0 };
Packet          *notifications[MAX_NOTIFICATIONS] = { 0 };

PendingRequest *createRequest(void)
{
    PendingRequest *request = NULL;

    for (int i = 0; i < MAX_PENDING_REQUESTS; i++) {
        if (pendingRequests[i] == NULL) {
            pendingRequests[i] = malloc(sizeof(*request));
            request = pendingRequests[i];
            if (request == NULL) {
                DBG_FATAL("malloc failed");
                break;
            }
            request->data = malloc(PENDING_REQUEST_BUFFER_SIZE);
            if (request->data == NULL) {
                DBG_FATAL("malloc failed");
                free(request);
                break;
            }
            request->id = i;
            request->size = 0;
            request->capacity = PENDING_REQUEST_BUFFER_SIZE;
            request->event = CreateEvent(NULL, FALSE, FALSE, NULL);
            request->mutex = CreateMutex(NULL, FALSE, NULL);
            break;
        }
    }

    return request;
}

void writeToRequest(PendingRequest *request, uint8_t *data, size_t size)
{
    if (request->capacity < request->size + size) {
        request->data = realloc(request->data, request->size + size);
    }
    memcpy(request->data + request->size, data, size);
}

void deleteRequest(PendingRequest **request)
{
    if (*request == NULL)
        return;
    pendingRequests[(*request)->id] = NULL;
    if ((*request)->data != NULL)
        free((*request)->data);
    free(*request);
    *request = NULL;
}