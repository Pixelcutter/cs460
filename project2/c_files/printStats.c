#include <stdio.h>
#include <string.h>
// queues, endTimeMillis, startTimeMillis
#include "../h_files/global.h"

// returns that average turnaround time of processes found in the done queue
// avg turnaround time = sum([proc.finishTime - proc.arrivalTime for proc in doneQueue])/doneQueue.length
double getAvgTurnaround(){
    process* proc = doneQueue->head;
    int totalTurnaround = 0;
    while(proc){
        totalTurnaround += proc->finishTimeMillis - proc->arrivalTimeMillis;
        proc = proc->nextProc;
    }
    return (double)totalTurnaround / doneQueue->length;
}

// returns average waiting time of processes found in done queue
// avg wait time = sum([proc.readyQueueWaitTime for proc in doneQueue])/doneQueue.length
double getAvgWaitTime(){
    process* proc = doneQueue->head;
    int totalWaitTime = 0;
    while(proc){
        totalWaitTime += proc->readyQueueWaitTime;
        proc = proc->nextProc;
    }
    return (double)totalWaitTime / doneQueue->length;
}

// prints final output
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
