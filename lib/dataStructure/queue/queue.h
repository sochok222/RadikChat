#ifndef RADIKCHAT_QUEUE_H
#define RADIKCHAT_QUEUE_H

// DESCRIPTION:
// Thread-safe queue data structure

#include <synchapi.h>

typedef struct sQueueNode
{
    int value;
    struct sQueueNode *next;
} QueueNode;

typedef struct sQueue
{
    QueueNode *front;
    QueueNode *back;
} Queue;

void    initQueue(Queue *queue);
bool    isQueueEmpty(Queue *queue);
bool    pushToQueue(Queue *queue, int value);
int     popFromQueue(Queue *queue); // poping from empty queue will return 0

#endif //RADIKCHAT_QUEUE_H
