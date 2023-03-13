/**/
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "utilFuncs.h"

long startTimeMillis;
queue readyQueue, ioQueue;

int main(int argc, char *argv[]){
    // timestamp marking when the thread started
    startTimeMillis = currentTimeMillis();
    readyQueue = initQueue();
    ioQueue = initQueue();

    char whatever[] = "proc 1 5 20 50 60 79 90";
    char* rest = whatever;

    char* popped = strtok_r(rest, " ", &rest);
    process* newProc = initProc(rest);

    readyQueue.head = readyQueue.tail = newProc;
    printf("ready queue head priority = %d\n", readyQueue.head->priority);

    freeProc(newProc);
}