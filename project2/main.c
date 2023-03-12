/**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "scheduling.h"

queue initQueue();
process* initProc(char* procLine);
long currentTimeMillis();
long startTimeMillis;

int main(int argc, char *argv[]){
    // timestamp marking when the thread started
    startTimeMillis = currentTimeMillis();

    char whatever[] = "proc 1 5 20 50 60 79 90";

    char* trash;
    char* popped;
    char* rest = whatever;

    process *newProc;
    popped = strtok_r(rest, " ", &rest);
    if(!strcmp(popped, "proc"))
        newProc = initProc(rest);
    else
        printf("not a process!\n");
}

queue initQueue(){
    queue newQueue = {
            .length = 0,
            .head = NULL,
            .tail = NULL
        };
    
    return newQueue;
}

void errExit(char* message){
    printf("ERROR: %s\n", message);
    exit(1);
}

long currentTimeMillis(){
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return currTime.tv_sec * 1000;
}

int burstPop(char* rest){
    int parsedNum;
    sscanf(rest, "%d", &parsedNum);
    return parsedNum;
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