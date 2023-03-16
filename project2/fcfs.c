#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "utilFuncs.h"

void* fcfsFunc(void* args){
    while(TRUE){
        if((procsSeen == procsCompleted) && parsingDone)
            break;
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        process* proc = dequeue(readyQueue);
        pthread_mutex_unlock(&readyQueueMutex);
        
        int nextBurst = proc->schedule[proc->nextIndex];
        proc->totalBurstTime += nextBurst;
        // printf("cpu thread sleeping for < %d > seconds\n", nextBurst);
        usleep(nextBurst * 1000);
        
        if(proc->nextIndex == proc->scheduleLen-1){
            procsCompleted++;
            proc->finishTimeMillis = currentTimeMillis();
            enqueue(doneQueue, proc);
            continue;
        }
        proc->nextIndex++;
        
        pthread_mutex_lock(&ioQueueMutex);
        enqueue(ioQueue, proc);
        pthread_mutex_unlock(&ioQueueMutex);
        pthread_cond_signal(&ioQueueCond);
    }
    printf("cpu thread is done\n");
}
