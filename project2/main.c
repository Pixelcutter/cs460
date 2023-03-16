/**/
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "fcfs.h"
#include "io.h"
#include "utilFuncs.h"
#include "parser.h"

int parsingDone, procsSeen, procsCompleted;
long startTimeMillis;
queue *readyQueue, *ioQueue, *doneQueue;
pthread_mutex_t readyQueueMutex, ioQueueMutex;
pthread_cond_t readyQueueCond, ioQueueCond;

int getThroughput(){
    return 42069;
}

double getAvgTurnaround(){
    process* proc = doneQueue->head;
    int totalTurnaround = 0;
    while(proc){
        totalTurnaround += proc->finishTimeMillis - proc->arrivalTimeMillis;
        proc = proc->nextProc;
    }
    return ( totalTurnaround / doneQueue->length );
}

int getAvgWaitTime(){
    process* proc = doneQueue->head;
    int totalWaitTime = 0;
    while(proc){
        totalWaitTime += ( proc->finishTimeMillis - proc->arrivalTimeMillis ) - proc->totalBurstTime;
        proc = proc->nextProc;
    }
    return ( totalWaitTime / doneQueue->length );
}

int main(int argc, char *argv[]){
    // initializing global variables
    startTimeMillis = currentTimeMillis();
    readyQueue = initQueue(), ioQueue = initQueue(), doneQueue = initQueue();
    parsingDone = FALSE;
    procsSeen = procsCompleted = 0;
    
    if(argc < 5 || argc > 7)
        errExit("Invalid number of arguments\nUsage: ./exec -alg [FCFS|SJF|PR|RR] [-quantum [integer(ms)]] -input [filename]");

    if(strcmp(argv[1], "-alg"))
        errExit("Invalid first argument. Expected: '-alg'");
    
    char* algStr = argv[2];
    char* fileName;
    int quantum;
    pthread_t cpuThread, parserThread, ioThread;

    if(!strcmp(algStr, "RR")){
        if(argc != 7 || strcmp(argv[3], "-quantum"))
            errExit("Expected '-quantum' flag for algorithm: 'RR'");

        quantum = strToInt(argv[4]);
        fileName = argv[6];
        printf("starting cpu thread with < RR >..\n");
        pthread_create(&parserThread, NULL, &parseFile, fileName);
    }
    else{
        fileName = argv[4];
        pthread_create(&parserThread, NULL, &parseFile, fileName);
        pthread_detach(parserThread);
        
        if(!strcmp(algStr, "FCFS")){
            printf("starting cpu thread with < FCFS >..\n");
            pthread_create(&cpuThread, NULL, &fcfsFunc, NULL);
        }
        else if(!strcmp(algStr, "PR")){
            printf("starting cpu thread with < PR >..\n");
        }
        else if(!strcmp(algStr, "SJF")){
            printf("starting cpu thread with < SJF >..\n");
        }
        else
            errExit("Scheduling algorithm not recognized");
    }

    pthread_create(&ioThread, NULL, &ioFunc, NULL);
    pthread_detach(parserThread);

    pthread_join(cpuThread, NULL);

    process* proc = doneQueue->head;
    printf("done queue length = %d\n", doneQueue->length);
    while(proc){
        printf("arrival time = %ld | finish time = %ld\n", proc->arrivalTimeMillis, proc->finishTimeMillis);
        proc = proc->nextProc;
    }

    printf("Input File Name                 : %s\n", fileName);
    if(!strcmp(algStr, "RR"))
        printf("CPU Scheduling Alg              : %s (%d)\n", algStr, quantum);
    else
        printf("CPU Scheduling Alg              : %s\n", algStr);
    printf("Throughput                      : %d\n", getThroughput());
    printf("Avg. Turnaround Time            : %f\n", getAvgTurnaround());
    printf("Avg. Waiting Time in Ready Queue: %d\n", getAvgWaitTime());
    // pthread_join(parser, NULL);

    // printf("starting to print queue\n");

    // process* proc;    
    // while(proc = dequeue(readyQueue), proc != NULL)
    //     printf("priority (main): %d\n", proc->priority);

}