#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "utilFuncs.h"
#include "sjf.h"
#include "pr.h"

void* ioFunc(void* args){
    while(TRUE){
        if((procsSeen == procsCompleted) && parsingDone)
            break;

        pthread_mutex_lock(&ioQueueMutex);
        while(ioQueue->head == NULL){
            pthread_cond_wait(&ioQueueCond, &ioQueueMutex);
        }
        process* proc = dequeue(ioQueue);
        pthread_mutex_unlock(&ioQueueMutex);
        
        // printf("IO thread sleeping for < %d > seconds\n", proc->schedule[proc->nextIndex]);
        usleep(proc->schedule[proc->nextIndex] * 1000);
        proc->nextIndex++;

        pthread_mutex_lock(&readyQueueMutex);

        enqueue(readyQueue, proc);

        pthread_mutex_unlock(&readyQueueMutex);
        pthread_cond_signal(&readyQueueCond);
    }
    return NULL;
}