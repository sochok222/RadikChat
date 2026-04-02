#ifndef RADIKCHAT_DELIVERYMANAGER_H
#define RADIKCHAT_DELIVERYMANAGER_H

#include "request.h"

#define MAX_PENDING_DELIVERIES  MAX_PENDING_REQUESTS

typedef PendingRequest PendingDelivery;

PendingRequest* (*createDelivery)(void) = createRequest;
void            (*deleteDelivery)(PendingDelivery**) = deleteRequest;
void            (*writeToDelivery)(PendingDelivery* delivery, uint8_t *data, size_t size) = writeToRequest;


extern PendingDelivery *pendingDeliveries[MAX_PENDING_DELIVERIES];

#endif //RADIKCHAT_DELIVERYMANAGER_H
