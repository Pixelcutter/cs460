/**/
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "fcfs.h"
#include "io.h"
#include "utilFuncs.h"
#include "parser.h"
#include "sjf.h"
#include "pr.h"
#include "rr.h"

int parsingDone, procsSeen, procsCompleted;
double startTimeMillis, endTimeMillis;
queue *readyQueue, *ioQueue, *doneQueue;
pthread_mutex_t readyQueueMutex, ioQueueMutex;
pthread_cond_t readyQueueCond, ioQueueCond;

int main(int argc, char *argv[]){
    // initializing global variables
    readyQueue = initQueue(), ioQueue = initQueue(), doneQueue = initQueue();
    parsingDone = FALSE;
    procsSeen = procsCompleted = 0;
    
    if(argc < 5 || argc > 7)
        errExit("Invalid number of arguments\nUsage: ./exec -alg [FCFS|SJF|PR|RR] [-quantum [integer(ms)]] -input [filename]");

    if(strcmp(argv[1], "-alg"))
        errExit("Invalid first argument. Expected: '-alg'");
    
    pthread_t cpuThread, parserThread, ioThread;
    char* algStr = argv[2];
    char* fileName;
    int quantum;
    
    // start time set after error checking for better accuracy
    startTimeMillis = currentTimeMillis();

    if(!strcmp(algStr, "RR")){
        if(argc != 7 || strcmp(argv[3], "-quantum"))
            errExit("Expected '-quantum' flag for algorithm: 'RR'");

        quantum = strToInt(argv[4]);
        fileName = argv[6];
        
        pthread_create(&parserThread, NULL, &parseFile, fileName);
        pthread_detach(parserThread);

        pthread_create(&cpuThread, NULL, &rrFunc, &quantum);
    }
    else{
        fileName = argv[4];
        pthread_create(&parserThread, NULL, &parseFile, fileName);
        pthread_detach(parserThread);

        if(!strcmp(algStr, "FCFS"))
            pthread_create(&cpuThread, NULL, &fcfsFunc, NULL);
        else if(!strcmp(algStr, "SJF"))
            pthread_create(&cpuThread, NULL, &sjfFunc, NULL);
        else if(!strcmp(algStr, "PR"))
            pthread_create(&cpuThread, NULL, &prFunc, NULL);
        else
            errExit("Scheduling algorithm not recognized");
    }

    pthread_create(&ioThread, NULL, &ioFunc, NULL);
    pthread_detach(ioThread);

    pthread_join(cpuThread, NULL);
    endTimeMillis = currentTimeMillis();

    printf("Input File Name                 : %s\n", fileName);
    if(!strcmp(algStr, "RR"))
        printf("CPU Scheduling Alg              : %s (%d)\n", algStr, quantum);
    else
        printf("CPU Scheduling Alg              : %s\n", algStr);
    printf("Throughput                      : %f\n", doneQueue->length / ( endTimeMillis - startTimeMillis ));
    printf("Avg. Turnaround Time            : %f\n", getAvgTurnaround());
    printf("Avg. Waiting Time in Ready Queue: %f\n", getAvgWaitTime());
}