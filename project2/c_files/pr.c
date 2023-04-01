#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// procsSeen, procsCompleted, cpuDone, parsingDone, queues, mutexes
#include "../h_files/global.h" 
// enqueue(), dequeue()
#include "../h_files/utilFuncs.h"

// searches q for the proc with the highest priority and then returns it
// acts like a max function on an unordered list
process* getHighestPrio(queue* q){
    process* proc = q->head;
    // if queue contains one proc: return proc and set head and tail to NULL
    if(q->head == q->tail){
        q->head = q->tail = NULL;
        proc->nextProc = proc->prevProc = NULL;
        return proc;
    }

    process* highestPrio = proc;
    // while not null: traverse linked list
    while(proc){
        if(proc->priority > highestPrio->priority)
            highestPrio = proc;
        proc = proc->nextProc;
    }

    if(highestPrio == q->head){ // highest prio proc was head of queue
        q->head = highestPrio->nextProc;
        highestPrio->nextProc->prevProc = NULL;
    }
    else if(highestPrio == q->tail){ // highest prio proc was tail of queue
        q->tail = highestPrio->prevProc;
        highestPrio->prevProc->nextProc = NULL;
    }
    else{ // proc was not head or tail of queue
        highestPrio->prevProc->nextProc = highestPrio->nextProc;
        highestPrio->nextProc->prevProc = highestPrio->prevProc;
    }   
    
    // removing dangling pointers
    highestPrio->nextProc = highestPrio->prevProc = NULL;
    q->length--;

    return highestPrio;
}


// function that is passed to a thread that acts as a cpu scheduler following
// a priority algorithm
void* prFunc(void* args){
    // while the ready queue has processes in it and the parser thread is still
    // running: pull things from ready queue, sleep x amount of milleseconds
    // and then insert process into io queue
    while(TRUE){
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->head == NULL){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        process* proc = getHighestPrio(readyQueue);
        pthread_mutex_unlock(&readyQueueMutex);
        
        int nextBurst = proc->schedule[proc->nextIndex];
        proc->totalBurstTime += nextBurst;
        usleep(nextBurst * 1000);
        
        // if last burst time was processed: increment procsCompleted and
        // add proc to the doneQueue
        if(proc->nextIndex == proc->scheduleLen-1){
            procsCompleted++;
            proc->finishTimeMillis = currentTimeMillis() - startTimeMillis;
            enqueue(doneQueue, proc);

            // if all procs have been processed and the parsing thread is done:
            // set cpuDone flag, signal io thread that all procs are done,
            // and then exit
            if(procsCompleted == procsSeen && parsingDone){
                cpuDone = TRUE;
                pthread_cond_signal(&ioQueueCond);
                break;
            }

            continue;
        }
        proc->nextIndex++; // increment index to location of next burst time
        
        // obtain io queue lock, add process to io queue
        // and signal when done
        pthread_mutex_lock(&ioQueueMutex);
        enqueue(ioQueue, proc);
        pthread_mutex_unlock(&ioQueueMutex);
        pthread_cond_signal(&ioQueueCond);
    }
    return NULL;
}