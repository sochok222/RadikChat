#ifndef RADIKCHAT_DELIVERYMANAGER_H
#define RADIKCHAT_DELIVERYMANAGER_H

#include "request.h"

#define MAX_PENDING_DELIVERIES  MAX_PENDING_REQUESTS

#define createDelivery(...) createRequest(__VA_ARGS__)
#define deleteDelivery(...) deleteRequest(__VA_ARGS__)

typedef PendingRequest PendingDelivery;

extern PendingDelivery pendingDeliveries[MAX_PENDING_DELIVERIES];

#endif //RADIKCHAT_DELIVERYMANAGER_H
