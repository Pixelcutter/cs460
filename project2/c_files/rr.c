#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"

void* rrFunc(void* p){
    int quantum = *((int*)p);

    while(TRUE){
        // catching when ready queue is empty but could still be added to by
        // io or parser threads
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            printf("CPU waiting...\n");
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
            printf("CPU done waiting...\n");
        }
        process* proc = dequeue(readyQueue);
        pthread_mutex_unlock(&readyQueueMutex);
        
        int nextBurst = proc->schedule[proc->nextIndex];
        
        if(nextBurst > quantum)
            nextBurst = quantum;
            
        proc->totalBurstTime += nextBurst;
        proc->schedule[proc->nextIndex] -= nextBurst;
        usleep(nextBurst * 1000);

        if(proc->schedule[proc->nextIndex] == 0){
            if(proc->nextIndex == proc->scheduleLen-1){
                procsCompleted++;
                proc->finishTimeMillis = currentTimeMillis() - startTimeMillis;
                enqueue(doneQueue, proc);
                
                if(procsCompleted == procsSeen && parsingDone){
                    cpuDone = TRUE;
                    pthread_cond_signal(&ioQueueCond);
                    break;
                }
            }
            else{
                proc->nextIndex++;
                
                pthread_mutex_lock(&ioQueueMutex);
                enqueue(ioQueue, proc);
                pthread_mutex_unlock(&ioQueueMutex);
                pthread_cond_signal(&ioQueueCond);
            }
        }
        else{
            enqueue(readyQueue, proc);
        }
    }
    // printf("------ CPU DONE -------\n");
    return NULL;
}