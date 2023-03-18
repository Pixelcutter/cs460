#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"


process* getHighestPrio(queue* q){
    process* proc = q->head;
    if(q->head == q->tail){
        q->head = q->tail = NULL;
        proc->nextProc = proc->prevProc = NULL;
        return proc;
    }

    process* highestPrio = proc;
    while(proc){
        if(proc->priority > highestPrio->priority)
            highestPrio = proc;
        proc = proc->nextProc;
    }

    if(highestPrio == q->head){
        q->head = highestPrio->nextProc;
        highestPrio->nextProc->prevProc = NULL;
    }
    else if(highestPrio == q->tail){
        q->tail = highestPrio->prevProc;
        highestPrio->prevProc->nextProc = NULL;
    }
    else{
        highestPrio->prevProc->nextProc = highestPrio->nextProc;
        highestPrio->nextProc->prevProc = highestPrio->prevProc;
    }   
    
    highestPrio->nextProc = highestPrio->prevProc = NULL;
    q->length--;

    // printf("highest prio = %d | the rest [ ", highestPrio->priority);
    // process* tmp = q->head;
    // while(tmp){
    //     printf("%d, ", tmp->priority);
    //     tmp = tmp->nextProc;
    // }
    // printf(" ]\n");

    return highestPrio;
}

void* prFunc(void* args){
    while(TRUE){
        // if((procsSeen == procsCompleted) && parsingDone)
        //     break;
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        // process* proc = dequeue(readyQueue);
        process* proc = getHighestPrio(readyQueue);
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
        pthread_mutex_unlock(&ioQueueMutex);
        pthread_cond_signal(&ioQueueCond);
    }
    return NULL;
}