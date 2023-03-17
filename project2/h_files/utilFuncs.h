#ifndef INITFUNCS_H 
#define INITFUNCS_H 

queue* initQueue();
void errExit(char* message);
double currentTimeMillis();
int strToInt(char* rest);
void freeProc(process *proc);
process* dequeue(queue* q);
void enqueue(queue* q, process* proc);

#endif
