#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// cpuDone, queues, mutexes
#include "../h_files/global.h"
// enqueue(), dequeue(), currentTimeMillis()
#include "../h_files/utilFuncs.h"

// function that is passed to a thread that acts as an io scheduler
void* ioFunc(void* args){
    // while the io queue has processes in it and the cpu thread is still
    // running: pull things from io queue, sleep x amount of milleseconds
    // and then insert process into ready queue
    while(TRUE){
        // catching when io queue is empty but a cpu thread is still
        // running that could add a process to it
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

        // obtain ready queue lock, add process to ready queue
        // and signal when done
        pthread_mutex_lock(&readyQueueMutex);
        // readyQueue entry timestamp
        proc->readyEnqueueTimeMillis = currentTimeMillis();
        enqueue(readyQueue, proc);
        pthread_mutex_unlock(&readyQueueMutex);
        pthread_cond_signal(&readyQueueCond);
    }
    return NULL;
}