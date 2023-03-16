#ifndef INITFUNCS_H 
#define INITFUNCS_H 

queue* initQueue();
void errExit(char* message);
long currentTimeMillis();
int strToInt(char* rest);
void freeProc(process *proc);
process* initProc(char* procLine);
process* dequeue(queue* q);
void enqueue(queue* q, process* proc);

#endif
