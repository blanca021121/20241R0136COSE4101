#include "config.h"
#include "process.h"
#include "scheduler.h"
#include "queue.h"
#include "metrics.h"

#include <stdio.h>
#include <pthread.h>

#define NUM_SCHEDULING 6

float avr_waiting_time[NUM_SCHEDULING]; 
float avr_turnaround_time[NUM_SCHEDULING];

volatile int io_thread_continue;

Process* process_list[NUM_PROCESS];
Process* original_process_list[NUM_PROCESS];

Queue readyQueue;
Queue waitingQueue;

int current_time = 0;

void free_process(Process** process) {
    if (process != NULL && *process != NULL) {
        free(*process);
        *process = NULL; // 포인터를 NULL로 설정
    }
}

int main() {
    char* algorithm_name[NUM_SCHEDULING] = { "FCFS Scheduling", "SJF Scheduling", "RR Scheduling", "Priority Scheduling", "Preemptive SJF Scheduling", "Preemptive Priority Scheduling" };
    int any;

    // CPU scheduling
    for (int algorithm = 0; algorithm < NUM_SCHEDULING; algorithm++) {
        io_thread_continue = 1;

        initializeQueue(&readyQueue);
        initializeQueue(&waitingQueue);

        if (algorithm == 0) {
            for (int i = 0; i < NUM_PROCESS; i++) {
                original_process_list[i] = create_process();
            }
        }

        // ready queue에 원본 프로세스 리스트 복사
        for (int i = 0; i < NUM_PROCESS; i++) {
            enqueue(&readyQueue, create_process_from(original_process_list[i]));
        }

        // ready queue에 있는 것들을 process_list로 옮김
        int i = 0;
        Process* current;

        while ((current = dequeue(&readyQueue)) != NULL) {
            process_list[i++] = current;
        }
        int n = i;

        current_time = 0;

        pthread_t cpu_thread, io_thread;

        switch (algorithm) {
            case 0:
                pthread_create(&cpu_thread, NULL, fcfs_scheduling, &n);
                break;
            case 1:
                pthread_create(&cpu_thread, NULL, sjf_scheduling, &n);
                break;
            case 2:
                pthread_create(&cpu_thread, NULL, rr_scheduling, &n);
                break;
            case 3:
                pthread_create(&cpu_thread, NULL, priority_scheduling, &n);
                break;
            case 4:
                pthread_create(&cpu_thread, NULL, preemptive_sjf_scheduling, &n);
                break;
            case 5:
                pthread_create(&cpu_thread, NULL, preemptive_priority_scheduling, &n);
                break;
            default:
                printf("Invalid choice\n");
                return 1;
            }
        printf("\nAlgorithm: %s\n", algorithm_name[algorithm]);
        pthread_create(&io_thread, NULL, io_scheduling, NULL);
        
        pthread_join(cpu_thread, NULL);
        usleep(10000);
        io_thread_continue = 0;
        pthread_join(io_thread, NULL);

        for (int k = 0; k < NUM_PROCESS; k++) {
            printf("PID: %d, Burst Time: %d, IO Burst: %d, Priority: %d, Arrival Time: %d, Data: %d\n",
                process_list[k]->pid, process_list[k]->original_burst_time, process_list[k]->io_burst, process_list[k]->priority, process_list[k]->arrival_time, process_list[k]->data);
        }

        metrics(algorithm);
        if (algorithm == 2 || algorithm==4 || algorithm==5) {
            rr_print_gantt_chart(MAX_COMPLETION_TIME);
        }
        else { printGanttChart(); }
       
        

        usleep(200000);

        for (int i = 0; i < NUM_PROCESS; i++) {
            free_process(&process_list[i]);
            process_list[i] = NULL;
        }

        
        printf("press num keys to skip");
        scanf("%d", &any);
    }
    
    printf("Comparing Scheduling\n");
    for (int k = 0; k < NUM_SCHEDULING; k++) {
        printf("%s: ", algorithm_name[k]);
        printf("Average Waiting Time - %f  ", avr_waiting_time[k]);
        printf("Average Turnaround Time - %f\n", avr_turnaround_time[k]);
    }

    return 0;
}