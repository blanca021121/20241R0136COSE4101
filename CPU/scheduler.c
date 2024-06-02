#include "scheduler.h"
#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define TIME_QUANTUM 3

extern Queue readyQueue;
extern Queue waitingQueue;
extern Process* process_list[NUM_PROCESS];

pthread_mutex_t readyQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitingQueueMutex = PTHREAD_MUTEX_INITIALIZER;


void sort_processes_by_arrival(Process* process_list[]) {
    int i, j;
    for (i = 0; i < NUM_PROCESS - 1; i++) {
        for (j = 0; j < NUM_PROCESS - i - 1; j++) {
            if (process_list[j]->arrival_time > process_list[j + 1]->arrival_time) {
                Process* temp = process_list[j];
                process_list[j] = process_list[j + 1];
                process_list[j + 1] = temp;
            }
        }
    }
}

// fcfs
void* fcfs_scheduling(void* arg) {
    int current_time = 0;
    int sum_burst_time = 0;

    sort_processes_by_arrival(process_list);

    if (NUM_PROCESS > 0) {
        current_time = process_list[0]->arrival_time;
    }

    for (int i = 0; i < NUM_PROCESS; i++) {
        if (current_time < process_list[i]->arrival_time) {
            current_time = process_list[i]->arrival_time;
        }

        process_list[i]->start_time = current_time;
        process_list[i]->completion_time = current_time + process_list[i]->cpu_burst;
        process_list[i]->turnaround_time = process_list[i]->completion_time - process_list[i]->arrival_time;
        process_list[i]->waiting_time = process_list[i]->start_time - process_list[i]->arrival_time;

        current_time += process_list[i]->cpu_burst;
    }

    for (int i = 0; i < NUM_PROCESS; i++) {
        printf("Running Process %d\n", process_list[i]->pid);
        usleep(process_list[i]->cpu_burst * 1000);

        if (process_list[i]->io_burst > 0) {
            pthread_mutex_lock(&waitingQueueMutex);
            enqueue(&waitingQueue, process_list[i]);
            pthread_mutex_unlock(&waitingQueueMutex);
        }

    }
    return NULL;
}

//sjf
void* sjf_scheduling(void* arg) {
    int current_time = 0;
    int processes_completed = 0;

    sort_processes_by_arrival(process_list); // 도착 시간에 따라 정렬

    while (processes_completed < NUM_PROCESS) {
        int shortest_job_index = -1;
        int shortest_job_burst = INT_MAX;

        // 현재 시간에 도착한 프로세스들 중에서 CPU 버스트 시간이 가장 짧은 프로세스를 선택
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->arrival_time <= current_time && process_list[i]->cpu_burst > 0) {
                if (process_list[i]->cpu_burst < shortest_job_burst) {
                    shortest_job_burst = process_list[i]->cpu_burst;
                    shortest_job_index = i;
                }
            }
        }

        if (shortest_job_index == -1) {
            // 실행 가능한 프로세스가 없는 경우, 현재 시간을 증가시킴
            current_time++;
            continue;
        }

        // 선택된 프로세스를 실행
        Process* current_process = process_list[shortest_job_index];
        current_process->start_time = current_time;
        current_process->completion_time = current_time + current_process->cpu_burst;
        current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
        current_process->waiting_time = current_process->start_time - current_process->arrival_time;

        printf("Running Process %d\n", current_process->pid);
        usleep(current_process->cpu_burst * 1000);

        if (current_process->io_burst > 0) {
            pthread_mutex_lock(&waitingQueueMutex);
            enqueue(&waitingQueue, current_process);
            pthread_mutex_unlock(&waitingQueueMutex);
        }

        current_time += current_process->cpu_burst;
        current_process->cpu_burst = 0; // 프로세스 완료
        processes_completed++;
    }

    usleep(1000);
    return NULL;
}

// RR scheduling

extern int* gantt_chart;
extern void initialize_gantt_chart(int size);
extern int find_max_completion_time();

void* rr_scheduling(void* arg) {
    int max_completion_time = find_max_completion_time();
    initialize_gantt_chart(max_completion_time + 1);  // 최대 완료 시간 +1로 초기화

    printf("time_quantum: %d\n", TIME_QUANTUM);
    int current_time = 0;
    int processes_completed = 0;

    sort_processes_by_arrival(process_list);

    while (processes_completed < NUM_PROCESS) {
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->cpu_burst > 0) {
                if (current_time < process_list[i]->arrival_time) {
                    current_time = process_list[i]->arrival_time;
                }

                if (process_list[i]->start_time == -1) {
                    process_list[i]->start_time = current_time;
                }

                int burst_time = (process_list[i]->cpu_burst > TIME_QUANTUM) ? TIME_QUANTUM : process_list[i]->cpu_burst;
                for (int t = current_time; t < current_time + burst_time; t++) {
                    if (t < max_completion_time) {  // 범위 확인
                        gantt_chart[t] = process_list[i]->pid;
                    }
                }

                current_time += burst_time;
                process_list[i]->cpu_burst -= burst_time;

                if (process_list[i]->cpu_burst == 0) {
                    process_list[i]->completion_time = current_time;
                    process_list[i]->turnaround_time = process_list[i]->completion_time - process_list[i]->arrival_time;
                    process_list[i]->waiting_time = process_list[i]->turnaround_time - process_list[i]->original_burst_time;
                    processes_completed++;
                }

                usleep(burst_time * 1000);

                if (process_list[i]->io_burst > 0 && process_list[i]->cpu_burst == 0) {
                    pthread_mutex_lock(&waitingQueueMutex);
                    enqueue(&waitingQueue, process_list[i]);
                    pthread_mutex_unlock(&waitingQueueMutex);
                }
            }
        }
    }
    return NULL;
}

// Priority scheduling - priority 값이 작을 수록 우선순위가 높음
// 우선 순위 같을 시 fcfs 사용
void* priority_scheduling(void* arg) {
    int current_time = 0;
    int processes_completed = 0;

    sort_processes_by_arrival(process_list);

    while (processes_completed < NUM_PROCESS) {
        Queue priorityQueue;
        initializeQueue(&priorityQueue);

        int highest_priority = INT_MAX;

        // 도착한 프로세스 중 가장 높은 우선순위 찾기
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->arrival_time <= current_time && process_list[i]->cpu_burst > 0) {
                if (process_list[i]->priority < highest_priority) {
                    highest_priority = process_list[i]->priority;
                }
            }
        }

        // 만약 실행할 수 있는 프로세스가 없다면 시간 증가
        if (highest_priority == INT_MAX) {
            current_time++;
            continue;
        }

        // 우선순위가 같은 모든 프로세스를 큐에 추가 - 도착 가능한 경우만
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->arrival_time <= current_time &&
                process_list[i]->cpu_burst > 0 &&
                process_list[i]->priority == highest_priority) {
                enqueue(&priorityQueue, process_list[i]);
            }
        }

        // 우선순위가 같은 프로세스를 FCFS 방식으로 처리
        while (!isEmpty(&priorityQueue)) {
            Process* process = dequeue(&priorityQueue);
            if (process->cpu_burst > 0) {
                if (current_time < process->arrival_time) {
                    current_time = process->arrival_time;
                }

                if (process->start_time == -1) {
                    process->start_time = current_time;
                }

                int burst_time = process->cpu_burst;
                current_time += burst_time;
                process->cpu_burst = 0; // 프로세스 완료

                process->completion_time = current_time;
                process->turnaround_time = process->completion_time - process->arrival_time;
                process->waiting_time = process->turnaround_time - process->original_burst_time;
                processes_completed++;

                printf("Running Process %d\n", process->pid);
                usleep(burst_time * 1000);

                if (process->io_burst > 0) {
                    pthread_mutex_lock(&waitingQueueMutex);
                    enqueue(&waitingQueue, process);
                    pthread_mutex_unlock(&waitingQueueMutex);
                }
            }
        }
    }


    return NULL;
}



// preemptive_sjf_scheduling
void* preemptive_sjf_scheduling(void* arg) {
    int max_completion_time = find_max_completion_time();
    initialize_gantt_chart(max_completion_time + 1);  // 최대 완료 시간 +1로 초기화

    int current_time = 0;
    int processes_completed = 0;

    while (processes_completed < NUM_PROCESS) {
        int min_remaining_time = INT_MAX;
        int min_index = -1;

        // 도착한 프로세스 중 버스트 시간이 가장 짧은 것 찾기
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->arrival_time <= current_time &&
                process_list[i]->cpu_burst > 0 &&
                process_list[i]->cpu_burst < min_remaining_time) {
                min_remaining_time = process_list[i]->cpu_burst;
                min_index = i;
            }
        }

        // 실행 가능한 프로세스가 없는 경우
        if (min_index == -1) {
            current_time++;
            continue;
        }

        // 시간 단위 1당 실행
        Process* current_process = process_list[min_index];

        if (current_process->start_time == -1) {
            current_process->start_time = current_time;
        }

        // 1만큼만 실행
        current_process->cpu_burst--;
        gantt_chart[current_time] = current_process->pid;
        current_time++;

        printf("Running Process %d\n", current_process->pid);
        usleep(1000);

        // 하나가 다 실행된 경우
        if (current_process->cpu_burst == 0) {
            current_process->completion_time = current_time;
            current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->original_burst_time;
            processes_completed++;

            if (current_process->io_burst > 0) {
                pthread_mutex_lock(&waitingQueueMutex);
                enqueue(&waitingQueue, current_process);
                pthread_mutex_unlock(&waitingQueueMutex);
            }
        }
    }

    return NULL;
}

// preemptive priority scheduling
void* preemptive_priority_scheduling(void* arg) {
    int max_completion_time = find_max_completion_time();
    initialize_gantt_chart(max_completion_time + 1);  // 완료 시간 +1로 초기화

    int current_time = 0;
    int processes_completed = 0;

    while (processes_completed < NUM_PROCESS) {
        int highest_priority = INT_MAX;
        int highest_priority_index = -1;

        // 도착한 프로세스 중 가장 높은 우선순위 프로세스 찾기
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (process_list[i]->arrival_time <= current_time && process_list[i]->cpu_burst > 0) {
                if (process_list[i]->priority < highest_priority) {
                    highest_priority = process_list[i]->priority;
                    highest_priority_index = i;
                }
            }
        }

        // 실행 가능한 프로세스가 없는 경우
        if (highest_priority_index == -1) {
            current_time++;
            continue;
        }

        // 시간 단위 1당 실행
        Process* current_process = process_list[highest_priority_index];

        if (current_process->start_time == -1) {
            current_process->start_time = current_time;
        }

        // 1만큼만 실행
        current_process->cpu_burst--;
        gantt_chart[current_time] = current_process->pid;  // Gantt 차트에 현재 프로세스 PID 기록
        current_time++;

        printf("Running Process %d\n", current_process->pid);
        usleep(1000);

        // 하나가 다 실행된 경우
        if (current_process->cpu_burst == 0) {
            current_process->completion_time = current_time;
            current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->original_burst_time;
            processes_completed++;

            if (current_process->io_burst > 0) {
                pthread_mutex_lock(&waitingQueueMutex);
                enqueue(&waitingQueue, current_process);
                pthread_mutex_unlock(&waitingQueueMutex);
            }
        }
    }

    return NULL;
}

// io scheduling
extern int io_thread_continue;

void* io_scheduling(void* arg) {
    while (io_thread_continue) {
        pthread_mutex_lock(&waitingQueueMutex);
        Process* current = dequeue(&waitingQueue);
        pthread_mutex_unlock(&waitingQueueMutex);

        if (current != NULL) {
            printf("Processing IO for Process %d\n", current->pid);
            usleep(current->io_burst * 1000);

            pthread_mutex_lock(&readyQueueMutex);
            enqueue(&readyQueue, current);
            pthread_mutex_unlock(&readyQueueMutex);
        }
        else {
            usleep(1000);  // No process, just wait
        }
    }
    return NULL;
}
