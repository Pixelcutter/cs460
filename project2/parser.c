#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "utilFuncs.h"

// initializes proccesses before they are added to the ready queue
// procLine will be a line from a supplied file
process* initProc(char* procLine){
    process *newProc = malloc(sizeof(process));
    
    // arrival time = ( thread start time in ms ) - ( current time in ms )
    newProc->arrivalTimeMillis = currentTimeMillis() - startTimeMillis;
    newProc->finishTimeMillis = -1; // acts as a flag
    newProc->totalBurstTime = 0;
    newProc->nextIndex = 0;
    newProc->prevProc = NULL;
    newProc->nextProc = NULL;
    
    char* rest = procLine;
    newProc->priority = strToInt(strtok_r(rest, " ", &rest));
    newProc->scheduleLen = strToInt(strtok_r(rest, " ", &rest));
    newProc->schedule = malloc(sizeof(int) * newProc->scheduleLen);

    char* popped;
    for(int i = 0; i < newProc->scheduleLen; i++){
        if((popped = strtok_r(rest, " ", &rest)), !popped)
            errExit("Invalid number of burst times found");

        newProc->schedule[i] = strToInt(popped);
    }
    return newProc;
}

void* parseFile(void* p){
    char* fileName = (char*) p;
    FILE* fp = fopen(fileName, "r");

    char* rest = NULL;
    size_t len = 0;
    ssize_t nRead;

    while(nRead = getline(&rest, &len, fp), nRead > 0){
        char* popped = strtok_r(rest, " ", &rest);

        if(!strcmp(popped, "stop"))
            break;
        
        if(!strcmp(popped, "sleep")){
            popped = strtok_r(rest, " ", &rest);
            int sleepTime = strToInt(popped);
            usleep(sleepTime * 1000);
            continue;
        }
        
        process* proc = initProc(rest);
        pthread_mutex_lock(&readyQueueMutex);

        enqueue(readyQueue, proc);

        pthread_mutex_unlock(&readyQueueMutex);
        pthread_cond_signal(&readyQueueCond);
        procsSeen++;
    }
    parsingDone = TRUE;
    return NULL;
}