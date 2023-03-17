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
        while(ioQueue->head == NULL){
            // printf("IO procsSeen: %d | procsCompleted: %d | cpuDone: %d | parserDone %d\n", procsSeen, procsCompleted, cpuDone, parsingDone);
            // printf("IO waiting...\n");
            pthread_cond_wait(&ioQueueCond, &ioQueueMutex);
            // printf("IO done waiting...\n");
            if(cpuDone){
                pthread_mutex_unlock(&ioQueueMutex);
                return NULL;
            }
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
    // printf("----- IO DONE -----\n");
    return NULL;
}