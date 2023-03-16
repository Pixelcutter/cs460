#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "utilFuncs.h"

void* parseFile(void* p){
    char* fileName = (char*) p;
    FILE* fp = fopen(fileName, "r");

    char* rest = NULL;
    size_t len = 0;
    ssize_t nRead;

    while(getline(&rest, &len, fp) > 0){
        char* popped = strtok_r(rest, " ", &rest);

        if(!strcmp(popped, "stop"))
            break;
        
        if(!strcmp(popped, "sleep")){
            popped = strtok_r(rest, " ", &rest);
            int sleepTime = strToInt(popped);
            usleep(sleepTime * 1000);
            continue;
        }
        
        enqueue(readyQueue, initProc(rest));
        procsSeen++;
    }
    parsingDone = TRUE;
}