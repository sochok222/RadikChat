#include "queue.h"
#include <stddef.h>
#include <stdlib.h>

void init_queue(Queue *queue)
{
    queue->front = NULL;
    queue->back = NULL;
}

bool is_queue_empty(Queue *queue)
{
    return queue->back == NULL;
}

bool push_to_queue(Queue *queue, int value)
{
    QueueNode *newNode = malloc(sizeof(*newNode));
    if (newNode == NULL) {
        return false;
    }
    newNode->value = value;
    newNode->next = NULL;

    if (queue->front == NULL) {
        queue->front = newNode;
        queue->back = newNode;
    } else {
        queue->front->next = newNode;
        queue->front = queue->front->next;
    }
    return true;
}

int pop_from_queue(Queue *queue)
{
    if (queue->back == NULL) {
        return -1;
    }

    int value = queue->back->value;
    QueueNode *temp = queue->back;
    queue->back = queue->back->next;
    if (queue->back == NULL) {
        queue->front = queue->front->next;
    }
    free(temp);

    return value;
}
