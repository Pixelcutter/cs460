#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// procsSeen, procsCompleted, cpuDone, parsingDone, queues, mutexes
#include "../h_files/global.h"
// enqueue(), dequeue(), currentTimeMillis()
#include "../h_files/utilFuncs.h"

// function that is passed to a thread that acts as a cpu scheduler following
// a first come first serve algorithm
void* fcfsFunc(void* args){
    // while the ready queue has processes in it and the parser thread is still
    // running: pull things from ready queue, sleep x amount of milleseconds
    // and then insert process into io queue
    while(TRUE){
        // catching when ready queue is empty but io or parser threads are still
        // running and could add to ready queue
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->length == 0){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        process* proc = dequeue(readyQueue);
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
