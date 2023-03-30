#ifndef GLOBAL_H 
#define GLOBAL_H

#include <pthread.h>

#define TRUE 1
#define FALSE 0

typedef struct process{
    int priority;
    int totalBurstTime;
    int readyQueueWaitTime;
    double arrivalTimeMillis;
    double finishTimeMillis;
    double readyEnqueueTimeMillis;

    int nextIndex;
    int scheduleLen;
    int *schedule;

    struct process *prevProc;
    struct process *nextProc;
} process;

typedef struct queue{
    int length;
    struct process *head;
    struct process *tail;
} queue;

extern queue* readyQueue;
extern queue* ioQueue;
extern queue* doneQueue;

extern int procsSeen;
extern int procsCompleted;
extern int parsingDone;
extern int cpuDone;

extern double startTimeMillis;
extern double endTimeMillis;

extern pthread_mutex_t readyQueueMutex;
extern pthread_mutex_t ioQueueMutex;
extern pthread_cond_t  readyQueueCond;
extern pthread_cond_t ioQueueCond;

#endif