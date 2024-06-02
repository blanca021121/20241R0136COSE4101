#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

void initializeQueue(Queue* q) {
    q->front = q->rear = NULL;
}

void enqueue(Queue* q, Process* p) {
    p->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = p;
    }
    else {
        q->rear->next = p;
        q->rear = p;
    }
}

Process* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;

    Process* temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    temp->next = NULL;
    return temp;
}

void printQueue(Queue* q) {
    Process* temp = q->front;
    while (temp != NULL) {
        printf("PID: %d, Burst Time: %d, IO Burst: %d, Priority: %d, Arrival Time: %d, Data: %d\n",
            temp->pid, temp->cpu_burst, temp->io_burst, temp->priority, temp->arrival_time, temp->data);
        temp = temp->next;
    }
}

int isEmpty(Queue* q) {
    return q->front == NULL;
}
