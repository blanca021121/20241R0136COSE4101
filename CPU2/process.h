#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"

typedef struct {
    int pid;
    int cpu_burst;
    int io_burst;
	int original_burst_time;
	int start_time;
    int arrival_time;
    int priority;
	int completion_time;
	int turnaround_time;
	int waiting_time;
	int data;
	
	struct Process* next;
} Process;

Process* create_process();
Process* create_process_from(Process* src);

#endif // PROCESS_H