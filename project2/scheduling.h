#ifndef SCHEDULING.H
#define SCHEDULING.H

typedef struct process{
    int priority;
    int totalBurstTime;
    int arrivalTimeMillis;
    int finishTimeMillis;
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

extern *queue readyQueue;
extern *queue ioQueue;

#endif