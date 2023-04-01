#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// parsingDone, queues
#include "../h_files/global.h"
// enqueue()
#include "../h_files/utilFuncs.h"

// initializes proccesses before they are added to the ready queue
// procLine will be a line from a supplied file
process* initProc(char* procLine){
    process *newProc = malloc(sizeof(process));
    
    // arrival time = ( current time in ms ) - ( thread start in ms )
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

// function passed to a thread that will will parse a file and manipulate the ready queue
void* parseFile(void* p){
    char* fileName = (char*) p;
    FILE* fp = fopen(fileName, "r");

    char* line = NULL;
    size_t len = 0;
    ssize_t nRead;
    char* popped;
    char* tmp;

    // while fp has lines of input: either sleep x, initialize a proc, or stop
    while(nRead = getline(&line, &len, fp), nRead > 0){
        // line saved to a temporary variable because it will be
        // unreachable (can't be freed) once changed by strtok_r
        tmp = line;
        popped = strtok_r(tmp, " ", &tmp);

        if(!strcmp(popped, "stop"))
            break;
        
        if(!strcmp(popped, "sleep")){
            popped = strtok_r(tmp, " ", &tmp);
            int sleepTime = strToInt(popped);
            usleep(sleepTime * 1000);
            continue;
        }

        // proc initialized and added to the ready queue
        process* proc = initProc(tmp);
        pthread_mutex_lock(&readyQueueMutex);
        enqueue(readyQueue, proc);
        procsSeen++;
        pthread_cond_signal(&readyQueueCond);
        pthread_mutex_unlock(&readyQueueMutex);
    }
    
    fclose(fp);
    free(line); // freeing reachable line
    parsingDone = TRUE;
    return NULL;
}