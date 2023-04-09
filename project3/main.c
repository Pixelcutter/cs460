/**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>


typedef struct threadInfo{
    int threadNum;
    char* filename;
} threadInfo;

char* appendBak(char* filename){
    int len = strlen(filename);
    char* newStr = calloc(len + 8 + 5, sizeof(char)); // len('.backup/) = 8, len('.bak\0') = 5
    strcat(newStr, ".backup/");
    strcat(newStr, filename);
    strcat(newStr, ".bak");

    return newStr;
}

int openFile(char* filename, int flags, int perms){
    int fd;
    if(fd = open(filename, flags, perms), fd < 0){
        printf("ERROR: %s\n", strerror(errno));
        printf("problem opening file '%s'\nABORTING...\n", filename);
    }
    return fd;
}

int writeFile(int src, int dest){
    int buffSize = 56, amtRead = 0;
    char buffer[buffSize];

    int totalBytesWritten = 0;
    while(amtRead = read(src, buffer, buffSize), amtRead > 0){
        totalBytesWritten += amtRead;
        write(dest, buffer, amtRead);
    }

    return totalBytesWritten;
}

void* copyFile(void* args){
    threadInfo *tInfo = (threadInfo*)args;
    char* backupFilePath = appendBak(tInfo->filename);
    int src, dest;

    src = openFile(tInfo->filename, O_RDONLY, 0);
    dest = openFile(backupFilePath, O_WRONLY|O_CREAT, 0666);
    int totalBytesWritten = writeFile(src, dest);

    printf("[thread %d] Copied %d bytes from %s to %s.bak\n", tInfo->threadNum, totalBytesWritten, tInfo->filename, tInfo->filename);

    close(src);
    close(dest);
    free(backupFilePath);
    free(tInfo);    
}


int main(int argc, char* argv[]){
    pthread_t t1;
    
    threadInfo *tInfo = malloc(sizeof(threadInfo));
    tInfo->threadNum = 1;
    tInfo->filename = argv[1];

    pthread_create(&t1, NULL, &copyFile, tInfo);
    pthread_join(t1, NULL);
}
