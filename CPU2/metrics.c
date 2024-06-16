#include "config.h"
#include "metrics.h"
#include "process.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

extern Process* process_list[];
extern float avr_waiting_time[];
extern float avr_turnaround_time[];



void metrics(int algorithm) {
    int total_waiting_time = 0;
    int total_turnaround_time = 0;

    for (int i = 0; i < NUM_PROCESS; i++) {
        total_waiting_time += process_list[i]->waiting_time;
        total_turnaround_time += process_list[i]->turnaround_time;
        printf("pid: %d\tstart_time: %d\twaiting_time: %d\tcompletion_time: %d\tturnaround_time: %d\n",
            process_list[i]->pid,
            process_list[i]->start_time,
            process_list[i]->waiting_time,
            process_list[i]->completion_time,
            process_list[i]->turnaround_time);
    }

    avr_waiting_time[algorithm] = (float)total_waiting_time / NUM_PROCESS;
    avr_turnaround_time[algorithm] = (float)total_turnaround_time / NUM_PROCESS;

    printf("Avr_Waiting_Time: %f\n", avr_waiting_time[algorithm]);
    printf("Avr_Turnaround_Time: %f\n", avr_turnaround_time[algorithm]);
}

void printGanttChart() {
    int min_start_time = INT_MAX;
    int max_completion_time = 0;

    // 최소 시작 시간과 최대 완료 시간 찾기
    for (int i = 0; i < NUM_PROCESS; i++) {
        if (process_list[i]->start_time < min_start_time) {
            min_start_time = process_list[i]->start_time;
        }
        if (process_list[i]->completion_time > max_completion_time) {
            max_completion_time = process_list[i]->completion_time;
        }
    }

    // 상단 시간 축 출력
    printf(" ");
    for (int time = min_start_time; time <= max_completion_time; time++) {
        printf("--");
    }
    printf("\n|");

    // 각 시간 단위마다 실행 중인 프로세스 찾기
    for (int time = min_start_time; time < max_completion_time; time++) {
        bool found = false;
        for (int i = 0; i < NUM_PROCESS; i++) {
            if (time >= process_list[i]->start_time && time < process_list[i]->completion_time) {
                printf("P%d |", process_list[i]->pid);
                found = true;
                break;
            }
        }
        if (!found) {
            printf("   |");
        }
    }
    printf("\n ");

    // 하단 시간 축 출력
    for (int time = min_start_time; time <= max_completion_time; time++) {
        printf("--");
    }
    printf("\n0");

    for (int time = min_start_time + 1; time <= max_completion_time; time++) {
        printf("  %d", time);
        if (time < 10) {
            printf("  ");
        }
    }
    printf("\n");
}

// rr 전용 gantt chart
int* gantt_chart;

void initialize_gantt_chart(int size) {
    gantt_chart = (int*)malloc(sizeof(int) * size);
    if (!gantt_chart) {
        fprintf(stderr, "Memory allocation failed for gantt chart.\n");
        exit(1);
    }
    for (int i = 0; i < size; i++) {
        gantt_chart[i] = -1;
    }
}

void rr_print_gantt_chart(int max_time) {
    printf("Gantt Chart:\n");

    // 상단 경계선 출력
    printf(" ");
    for (int time = 0; time < max_time; time++) {
        printf("----");
    }
    printf("\n|");

    // 간트 차트 내용 출력
    for (int time = 0; time < max_time; time++) {
        if (gantt_chart[time] != -1) {
            printf("P%-2d|", gantt_chart[time]);
        }
        else {
            printf("   |");
        }
    }
    printf("\n ");

    // 하단 경계선 출력
    for (int time = 0; time < max_time; time++) {
        printf("----");
    }
    printf("\n");

    // 시간 축 출력
    printf("  ");
    for (int time = 0; time < max_time; time++) {
        printf("%-3d ", time);
    }
    printf("\n");
}