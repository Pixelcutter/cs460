/**/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../h_files/global.h"
#include "../h_files/fcfs.h"
#include "../h_files/io.h"
#include "../h_files/utilFuncs.h"
#include "../h_files/parser.h"
#include "../h_files/sjf.h"
#include "../h_files/pr.h"
#include "../h_files/rr.h"
#include "../h_files/printStats.h"

int parsingDone, cpuDone, procsSeen, procsCompleted;
double startTimeMillis, endTimeMillis;
queue *readyQueue, *ioQueue, *doneQueue;
pthread_mutex_t readyQueueMutex, ioQueueMutex;
pthread_cond_t readyQueueCond, ioQueueCond;

int main(int argc, char *argv[]){
    // initializing global variables
    readyQueue = initQueue(), ioQueue = initQueue(), doneQueue = initQueue();
    parsingDone = cpuDone = FALSE;
    procsSeen = procsCompleted = 0;

    pthread_mutex_init(&readyQueueMutex, NULL);
    pthread_mutex_init(&ioQueueMutex, NULL);
    pthread_cond_init(&readyQueueCond, NULL);
    pthread_cond_init(&ioQueueCond, NULL);
    
    if(argc < 5 || argc > 7)
        errExit("Invalid number of arguments\nUsage: ./exec -alg [FCFS|SJF|PR|RR] [-quantum [integer(ms)]] -input [filename]");

    if(strcmp(argv[1], "-alg"))
        errExit("Invalid first argument. Expected: '-alg'");
    char* algStr = argv[2];
    
    // start time set after error checking for better accuracy
    startTimeMillis = currentTimeMillis();

    pthread_t cpuThread, parserThread, ioThread;
    char* fileName;
    int quantum;

    if(!strcmp(algStr, "RR")){
        if(argc != 7)
            errExit("Not enough arguments\nUsage: ./exec -alg RR -quantum [integer(ms)] -input [filename]");

        if(strcmp(argv[3], "-quantum"))
            errExit("Expected '-quantum' flag for algorithm: 'RR'");

        if(strcmp(argv[5], "-input"))
            errExit("Expected option '-input'\nUsage: ./exec -alg RR -quantum [integer(ms)] -input [filename]");

        quantum = strToInt(argv[4]);
        fileName = argv[6];
        
        pthread_create(&parserThread, NULL, &parseFile, fileName);
        pthread_detach(parserThread);

        pthread_create(&cpuThread, NULL, &rrFunc, &quantum);
    }
    else{
        if(strcmp(argv[3], "-input"))
            errExit("Expected option '-input'\nUsage: ./exec -alg [FCFS|SJF|PR] -input [filename]");

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
    printStats(fileName, algStr, quantum);

    pthread_mutex_destroy(&readyQueueMutex);
    pthread_mutex_destroy(&ioQueueMutex);
    pthread_cond_destroy(&readyQueueCond);
    pthread_cond_destroy(&ioQueueCond);

    freeQueues();
}