#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "global.h"

// initializes queue data structures
queue initQueue(){
    queue newQueue = {
            .length = 0,
            .head = NULL,
            .tail = NULL
        };
    
    return newQueue;
}

// convenient error handler function
void errExit(char* message){
    printf("ERROR: %s\n", message);
    exit(1);
}

// gets the current time in milliseconds
long currentTimeMillis(){
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return currTime.tv_sec * 1000;
}

// converts strings to int
int strToInt(char* rest){
    int parsedNum;
    sscanf(rest, "%d", &parsedNum);
    return parsedNum;
}

// Used to free proc memory at program end
void freeProc(process *proc){
    free(proc->schedule);
    free(proc);
}

// initializes proccesses before they are added to the ready queue
// proLine will be a line from a supplied file
process* initProc(char* procLine){
    process *newProc = malloc(sizeof(process));
    
    // arrival time = ( thread start time in ms ) - ( current time in ms )
    newProc->arrivalTimeMillis = startTimeMillis - currentTimeMillis();
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
        // printf("i = %d\n", newProc->schedule[i]);
    }
    // printf("arrival time = %ld\n", newProc->arrivalTimeMillis);
    return newProc;
}