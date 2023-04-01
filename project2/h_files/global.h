#ifndef GLOBAL_H 
#define GLOBAL_H

#include <pthread.h>

#define TRUE 1
#define FALSE 0

typedef struct process{
    int priority;                   // used by PR algorithm
    int totalBurstTime;             // incremented whenever a cpu burst is found in schedule
    int readyQueueWaitTime;         // incremented whenever dequeued from readyQueue
    double arrivalTimeMillis;       // set when initialized
    double finishTimeMillis;        // set when finished in cpu thread
    double readyEnqueueTimeMillis;  // set whenever pushed into readyQueue

    int nextIndex;                  // next schedule index to be processed
    int scheduleLen;                // length of schedule
    int *schedule;                  // parsed schedule

    struct process *prevProc;
    struct process *nextProc;
} process;

typedef struct queue{
    int length;                     // length of queue
    struct process *head;
    struct process *tail;
} queue;

extern queue* readyQueue;
extern queue* ioQueue;
extern queue* doneQueue;            // holds processes that have been fully processed by cpu thread

extern int procsSeen;               // number of processes seen by parser thread
extern int procsCompleted;          // number of processes finished in cpu thread
extern int parsingDone;             // flag indicating whether or not parsing is done
extern int cpuDone;                 // flag indicating whether or not cpu is done with all processes

extern double startTimeMillis;      // start of program time in ms
extern double endTimeMillis;        // end of program time in ms

extern pthread_mutex_t readyQueueMutex;
extern pthread_mutex_t ioQueueMutex;
extern pthread_cond_t  readyQueueCond;
extern pthread_cond_t ioQueueCond;

#endif