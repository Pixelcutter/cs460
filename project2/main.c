/**/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "global.h"
#include "utilFuncs.h"
#include "parser.h"

int parsingDone, procsSeen, procsCompleted;
long startTimeMillis;
queue *readyQueue, *ioQueue, *doneQueue;

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
    pthread_t cpuThread, parserThread, ioQueue;

    if(!strcmp(algStr, "RR")){
        if(argc != 7 || strcmp(argv[3], "-quantum"))
            errExit("Expected '-quantum' flag for algorithm: 'RR'");

        int quantum = strToInt(argv[4]);
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
        
    


    // argv[3] = '-input'
    // char* fileName = argv[4];
    // pthread_t parser;
    // pthread_create(&parser, NULL, &parseFile, fileName);
    // pthread_join(parser, NULL);

    // printf("starting to print queue\n");

    // process* proc;    
    // while(proc = dequeue(readyQueue), proc != NULL)
    //     printf("priority (main): %d\n", proc->priority);

}