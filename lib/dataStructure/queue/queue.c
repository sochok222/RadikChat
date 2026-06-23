#include "queue.h"
#include <stddef.h>
#include <stdlib.h>

void initQueue(Queue *queue)
{
    queue->front = NULL;
    queue->back = NULL;
}

bool isQueueEmpty(Queue *queue)
{
    bool isEmpty = queue->back == NULL;
    return isEmpty;
}

bool pushToQueue(Queue *queue, int value)
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

int popFromQueue(Queue *queue)
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
