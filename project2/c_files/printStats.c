#include <stdio.h>
#include <string.h>
#include "../h_files/global.h"

double getAvgTurnaround(){
    process* proc = doneQueue->head;
    int totalTurnaround = 0;
    while(proc){
        // printf("arrival: %f | finish: %f | burst: %d\n", proc->arrivalTimeMillis, proc->finishTimeMillis, proc->totalBurstTime);
        totalTurnaround += proc->finishTimeMillis - proc->arrivalTimeMillis;
        proc = proc->nextProc;
    }
    return (double)totalTurnaround / doneQueue->length;
}

double getAvgWaitTime(){
    process* proc = doneQueue->head;
    int totalWaitTime = 0;
    while(proc){
        totalWaitTime += proc->readyQueueWaitTime;
        proc = proc->nextProc;
    }
    return (double)totalWaitTime / doneQueue->length;
}

void printStats(char* fileName, char* algStr, int quantum){
    printf("Input File Name                 : %s\n", fileName);
    if(!strcmp(algStr, "RR"))
        printf("CPU Scheduling Alg              : %s (%d)\n", algStr, quantum);
    else
        printf("CPU Scheduling Alg              : %s\n", algStr);
    printf("Throughput                      : %f\n", doneQueue->length / ( endTimeMillis - startTimeMillis ));
    printf("Avg. Turnaround Time            : %f\n", getAvgTurnaround());
    printf("Avg. Waiting Time in Ready Queue: %f\n", getAvgWaitTime());
}
