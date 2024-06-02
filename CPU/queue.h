#ifndef QUEUE_H
#define QUEUE_H

#include "process.h"z

typedef struct {
    Process *front;
    Process *rear;
} Queue;

void initializeQueue(Queue *q);
void enqueue(Queue *q, Process *p);
Process* dequeue(Queue *q);
void printQueue(Queue *q);
int isEmpty(Queue *q);
void destroyQueue(Queue *q);

#endif