#include "process.h"
#include <stdlib.h>

Process* create_process_from(Process* src) {
    Process* p = malloc(sizeof(Process));
    if (p == NULL) return NULL;

    p->pid = src->pid;
    p->cpu_burst = src->cpu_burst;
    p->io_burst = src->io_burst;
    p->original_burst_time = src->original_burst_time;
    p->start_time = src->start_time;
    p->arrival_time = src->arrival_time;
    p->priority = src->priority;
    p->completion_time = src->completion_time;
    p->turnaround_time = src->turnaround_time;
    p->waiting_time = src->waiting_time;
    p->data = src->data;
    p->next = NULL;

    return p;
}

Process* create_process() {
	Process *p = malloc(sizeof(Process));
	if (p == NULL) return NULL;
	 
    p->pid = rand()%100+1;
    p->cpu_burst = rand()%5+1;
    p->io_burst = rand()%5+1; //입출력에 걸리는 시간(그동안 waiting queue로 가고 끝나야 ready queue)
    p->original_burst_time=p->cpu_burst;
    p->start_time = -1;
    p->arrival_time = rand()%5;
    p->priority = rand() % NUM_PROCESS + 1;
    p->completion_time;
    p->turnaround_time;
    p->waiting_time;
    p->data = rand()%50;
    p->next = NULL;
	
    struct Process* next=NULL;

	return p;
}

