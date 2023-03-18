#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"

process* getShortest(queue* q){
    process* proc = q->head;
    if(q->head == q->tail){
        q->head = q->tail = NULL;
        proc->nextProc = proc->prevProc = NULL;
        return proc;
    }

    process* lowest = proc;
    while(proc){
        if(proc->schedule[proc->nextIndex] < lowest->schedule[lowest->nextIndex])
            lowest = proc;
        proc = proc->nextProc;
    }

    if(lowest == q->head){
        q->head = lowest->nextProc;
        lowest->nextProc->prevProc = NULL;
    }
    else if(lowest == q->tail){
        q->tail = lowest->prevProc;
        lowest->prevProc->nextProc = NULL;
    }
    else{
        lowest->prevProc->nextProc = lowest->nextProc;
        lowest->nextProc->prevProc = lowest->prevProc;
    }   
    
    lowest->nextProc = lowest->prevProc = NULL;
    q->length--;

    // printf("shortest time = %d | the rest [ ", lowest->schedule[lowest->nextIndex]);
    // process* tmp = q->head;
    // while(tmp){
    //     printf("%d, ", tmp->schedule[tmp->nextIndex]);
    //     tmp = tmp->nextProc;
    // }
    // printf(" ]\n");

    return lowest;
}

void* sjfFunc(void* args){
    while(TRUE){
        // if((procsSeen == procsCompleted) && parsingDone)
        //     break;
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        process* proc = getShortest(readyQueue);
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