/**/
#include <stdlib.h>
#include <unistd.h>
#include "scheduling.h"

int burstPop(char* rest){
    int parsedNum;
    sscanf(rest, "%d", &parsedNum);
    return parsedNum;
}

queue initQueue(){
    queue newQueue = {
            .length = 0,
            .head = NULL,
            .tail = NULL
          };
    
    return newQueue;
}

process* initProc(char* procLine){
    process *newProc = malloc(sizeof(process));
    
    newProc->totalBurstTime = 0;
    newProc->nextIndex = 0;
    newProc->prevProc = NULL;
    newProc->nextProc = NULL;
    
    char* rest = procLine;
    newProc->priority = burstPop(strtok_r(rest, " ", &rest));
    newProc->scheduleLen = burstPop(strtok_r(rest, " ", &rest));
    newProc->schedule = malloc(sizeof(int) * newProc->scheduleLen);

    char* popped;
    for(int i = 0; i < newProc->scheduleLen; i++){
        if((popped = strtok_r(rest, " ", &rest)), !popped)
            errExit("Invalid number of burst times found");

        newProc->schedule[i] = burstPop(popped);
        // printf("i = %d\n", newProc->schedule[i]);
    }

    // arrival time = ( thread start time in ms ) - ( current time in ms )
    newProc->arrivalTimeMillis = startTimeMillis - currentTimeMillis();
    // printf("arrival time = %ld\n", newProc->arrivalTimeMillis);

    return newProc;
}
