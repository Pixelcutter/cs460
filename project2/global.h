#ifndef GLOBAL_H 
#define GLOBAL_H 

typedef struct process{
    int priority;
    int totalBurstTime;
    long arrivalTimeMillis;
    long finishTimeMillis;

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

enum ALGO { FCFS, SJF, PR, RR };

extern queue readyQueue;
extern queue ioQueue;
extern int algType;
extern long startTimeMillis;

#endif