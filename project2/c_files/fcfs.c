#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"

void* fcfsFunc(void* args){
    while(TRUE){
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
            // printf("CPU is done waiting..\n");
        }
        process* proc = dequeue(readyQueue);
        pthread_mutex_unlock(&readyQueueMutex);
        
        int nextBurst = proc->schedule[proc->nextIndex];
        proc->totalBurstTime += nextBurst;
        // printf("cpu thread sleeping for < %d > seconds\n", nextBurst);
        usleep(nextBurst * 1000);
        
        if(proc->nextIndex == proc->scheduleLen-1){
            procsCompleted++;
            proc->finishTimeMillis = currentTimeMillis() - startTimeMillis;
            enqueue(doneQueue, proc);

            if(procsCompleted == procsSeen && parsingDone){
                cpuDone = TRUE;
                pthread_cond_signal(&ioQueueCond);
                break;
            }

            continue;
        }
        proc->nextIndex++;
        
        pthread_mutex_lock(&ioQueueMutex);
        enqueue(ioQueue, proc);
        pthread_cond_signal(&ioQueueCond);
        pthread_mutex_unlock(&ioQueueMutex);
    }
    cpuDone = TRUE;
    return NULL;
}
