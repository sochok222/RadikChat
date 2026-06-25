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

inline void    init_queue(Queue *queue);
inline bool    is_queue_empty(Queue *queue);
bool    push_to_queue(Queue *queue, int value);
int     pop_from_queue(Queue *queue); // poping from empty queue will return 0

#endif //RADIKCHAT_QUEUE_H
