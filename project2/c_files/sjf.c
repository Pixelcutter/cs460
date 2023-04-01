#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// procsSeen, procsCompleted, cpuDone, parsingDone, queues, mutexes
#include "../h_files/global.h"
// enqueue(), dequeue(), currentTimeMillis()
#include "../h_files/utilFuncs.h"

// searches q for the proc with the lowest burst time and then returns it
// acts like a min function on an unordered list
process* getShortest(queue* q){
    process* proc = q->head;
    // if queue contains one proc: return proc and set head and tail to NULL
    if(q->head == q->tail){
        q->head = q->tail = NULL;
        proc->nextProc = proc->prevProc = NULL;
        return proc;
    }

    process* lowest = proc;
    // while not null: traverse linked list
    while(proc){
        if(proc->schedule[proc->nextIndex] < lowest->schedule[lowest->nextIndex])
            lowest = proc;
        proc = proc->nextProc;
    }

    if(lowest == q->head){ // lowest burst time proc was head of queue
        q->head = lowest->nextProc;
        lowest->nextProc->prevProc = NULL;
    }
    else if(lowest == q->tail){ // lowest burst time proc was tail of queue
        q->tail = lowest->prevProc;
        lowest->prevProc->nextProc = NULL;
    }
    else{ // proc not head or tail of queue
        lowest->prevProc->nextProc = lowest->nextProc;
        lowest->nextProc->prevProc = lowest->prevProc;
    }   
    
    // removing dangling pointers
    lowest->nextProc = lowest->prevProc = NULL;
    q->length--;

    return lowest;
}


// function that is passed to a thread that acts as a cpu scheduler following
// a shortest job first algorithm
void* sjfFunc(void* args){
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
        process* proc = getShortest(readyQueue);
        // time diff between ready queue entry and exit added to wait time
        proc->readyQueueWaitTime += currentTimeMillis() - proc->readyEnqueueTimeMillis;
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