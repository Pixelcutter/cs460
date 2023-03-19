#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"
#include "../h_files/sjf.h"
#include "../h_files/pr.h"

void* ioFunc(void* args){
    while(TRUE){
        pthread_mutex_lock(&ioQueueMutex);
        while(ioQueue->length == 0){
            pthread_cond_wait(&ioQueueCond, &ioQueueMutex);
            if(cpuDone){
                pthread_mutex_unlock(&ioQueueMutex);
                return NULL;
            }
        }
        process* proc = dequeue(ioQueue);
        pthread_mutex_unlock(&ioQueueMutex);
        
        usleep(proc->schedule[proc->nextIndex] * 1000);
        proc->nextIndex++;

        pthread_mutex_lock(&readyQueueMutex);

        enqueue(readyQueue, proc);

        pthread_mutex_unlock(&readyQueueMutex);
        pthread_cond_signal(&readyQueueCond);
    }
    return NULL;
}