#include "process.h"
#include <stdlib.h>


Process* create_process() {
	Process *p = malloc(sizeof(Process));
	if (p == NULL) return NULL;
	 
    p->pid = rand()%100+1;
    p->cpu_burst = rand()%5+1;
    p->io_burst = rand()%5+1; //����¿� �ɸ��� �ð�(�׵��� waiting queue�� ���� ������ ready queue)
    p->original_burst_time=p->cpu_burst;
    p->start_time = -1;
    p->arrival_time = rand()%5;
    p->priority = rand() % NUM_PROCESS + 1;
    p->completion_time;
    p->turnaround_time;
    p->waiting_time;
    p->data = rand()%50;
	
    struct Process* next=NULL;

	return p;
}

