/**/
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "utilFuncs.h"

int algType;
long startTimeMillis;
queue readyQueue, ioQueue;

int main(int argc, char *argv[]){
    // timestamp marking when the thread started
    startTimeMillis = currentTimeMillis();
    readyQueue = initQueue();
    ioQueue = initQueue();
    
    if(argc < 5 || argc > 7)
        errExit("Invalid number of arguments\nUsage: ./exec -alg [FCFS|SJF|PR|RR] [-quantum [integer(ms)]] -input [filename]");

    if(strcmp(argv[1], "-alg"))
        errExit("Invalid first argument. Expected: '-alg'");

    char* alg = argv[2];

    if(!strcmp(alg, "RR"))
        if(argc != 7 || strcmp(argv[3], "-quantum"))
            errExit("Expected '-quantum' flag for algorithm: 'RR'");

    // char whatever[] = "proc 1 5 20 50 60 79 90";
    // char* rest = whatever;

    // char* popped = strtok_r(rest, " ", &rest);
    // process* newProc = initProc(rest);

    // readyQueue.head = readyQueue.tail = newProc;
    // printf("ready queue head priority = %d\n", readyQueue.head->priority);

    // freeProc(newProc);
}