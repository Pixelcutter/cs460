#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../h_files/global.h"
#include "../h_files/utilFuncs.h"

// function that is passed to a thread that acts as a cpu scheduler following
// a round robin algorithm
void* rrFunc(void* p){
    int quantum = *((int*)p);

    // while the ready queue has processes in it and the parser thread is still
    // running: pull things from ready queue, sleep x amount of milleseconds
    // and then insert process into io queue 
    while(TRUE){
        // catching when ready queue is empty but could still be added to by
        // io or parser threads
        pthread_mutex_lock(&readyQueueMutex);
        while(readyQueue->length == 0){
            pthread_cond_wait(&readyQueueCond, &readyQueueMutex);
        }
        process* proc = dequeue(readyQueue);
        pthread_mutex_unlock(&readyQueueMutex);
        
        int nextBurst = proc->schedule[proc->nextIndex];
        
        if(nextBurst > quantum)
            nextBurst = quantum;
            
        proc->totalBurstTime += nextBurst;
        proc->schedule[proc->nextIndex] -= nextBurst;
        usleep(nextBurst * 1000);

        // if burst time has been exhausted: check if on last burst time
        if(proc->schedule[proc->nextIndex] == 0){
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
            }
            else{ // add proc to io queue if not last burst time
                proc->nextIndex++;
                
                // obtain io queue lock, add process to io queue
                // and signal when done
                pthread_mutex_lock(&ioQueueMutex);
                enqueue(ioQueue, proc);
                pthread_mutex_unlock(&ioQueueMutex);
                pthread_cond_signal(&ioQueueCond);
            }
        }
        else{
            // if proc not done and current burst time != 0:
            // obtain ready queue lock and add process to ready queue
            pthread_mutex_lock(&readyQueueMutex);
            enqueue(readyQueue, proc);
            pthread_mutex_unlock(&readyQueueMutex);
        }
    }
    return NULL;
}