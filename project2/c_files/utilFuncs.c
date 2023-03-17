#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "../h_files/global.h"

// initializes queue data structures
queue* initQueue(){
    queue *newQueue = malloc(sizeof(queue));
    newQueue->length = 0;
    newQueue->head = newQueue->tail = NULL;

    return newQueue;
}

// convenient error handler function
void errExit(char* message){
    printf("ERROR: %s\n", message);
    exit(1);
}

// gets the current time in milliseconds
double currentTimeMillis(){
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    double timeInMillis = (currTime.tv_sec * 1000) + (double)currTime.tv_usec/1000;
    return timeInMillis;
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

process* dequeue(queue* q){
    if(q->head == NULL)
        return NULL;

    process* proc = q->head;
    if(q->head == q->tail)
        q->head = q->tail = NULL;
    else{
        q->head = q->head->nextProc;
        q->head->prevProc = NULL;
    }
    proc->nextProc = proc->prevProc = NULL;
    q->length--;
    return proc;
}

void enqueue(queue* q, process* proc){
    if(q->head == NULL){
        q->head = proc;
        proc->prevProc = NULL;
    }
    else{
        proc->prevProc = q->tail;
        q->tail->nextProc = proc;
        proc->nextProc = NULL;
    }
    q->tail = proc;
    q->length++;
}