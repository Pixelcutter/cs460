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
#include <sys/stat.h>

int threadNum = 0;
pthread_mutex_t threadNum_mutex;

// consider splitting this up into dir_threadInfo and file_threadInfo
typedef struct threadInfo{
    int threadNum;
    char* filename;
    char* rootDir;
    char* backupPath;
    char* backupFilePath;
} threadInfo;

char* appendBackup(char* filePath){
    int len = strlen(filePath);
    char* newStr = calloc(len + 9, sizeof(char)); // len('/.backup\0') = 9
    strcat(newStr, filePath);
    strcat(newStr, "/.backup");

    return newStr;
}

char* appendBak(char* filename){}

threadInfo* initThreadInfo(char* filename, char* rootDir){
    threadInfo* newInfo = malloc(sizeof(threadInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    newInfo->rootDir = rootDir;
    newInfo->backupPath = appendBackup(rootDir);
    return newInfo;
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

    return NULL;
}

char* buildPath(char* rootDir, char* dName){
    char* path = calloc(strlen(rootDir) + strlen(dName) + 2, sizeof(char));
    strcat(path, rootDir);
    strcat(path, "/");
    strcat(path, dName);

    return path;
}

void* listFiles(void* args) {
    threadInfo* dtInfo;
    threadInfo* tInfo = (threadInfo*)args;
    char* dirname = tInfo->rootDir;
    printf("dirname: %s\n", dirname);
    
    DIR* dir = opendir(dirname);
    if (dir == NULL)
        return NULL;

    char* backupPath = tInfo->backupPath;
    if(mkdir(backupPath, 0666) < 0 && errno != EEXIST)
        printf("mkdir error: %s\n", strerror(errno));

    DIR* backupDir;
    backupDir = opendir(backupPath);
    // printf("backupDir: %s\n", backupPath);
    if(backupDir == NULL)
        return NULL;

    struct dirent* entity;
    entity = readdir(dir);
    while (entity = readdir(dir), entity) {
        printf("%hhd %s/%s\n", entity->d_type, dirname, entity->d_name);
        if(entity->d_type == DT_DIR){
            if(!strcmp(entity->d_name, ".") || !strcmp(entity->d_name, "..") || !strcmp(entity->d_name, ".backup"))
                continue;

            char* rootDir = buildPath(dirname, entity->d_name);
            dtInfo = initThreadInfo(NULL, rootDir);
            pthread_t thread;
            pthread_create(&thread, NULL, &listFiles, dtInfo);
            pthread_join(thread, NULL);
            
            free(rootDir);
            free(dtInfo->backupPath);
            free(dtInfo);
        }
        else{

        }
    }

    closedir(dir);
    closedir(backupDir);
}

int main(int argc, char* argv[]){
    pthread_mutex_init(&threadNum_mutex, NULL);
    threadInfo* mainInfo = initThreadInfo(NULL, ".");

    pthread_t t1;
    
    // threadInfo *tInfo = malloc(sizeof(threadInfo));
    // tInfo->threadNum = 1;
    // tInfo->filename = argv[1];

    pthread_create(&t1, NULL, &listFiles, mainInfo);
    pthread_join(t1, NULL);

    // listFiles(mainInfo->filename);
    printf("id: %d | name: %s\n", mainInfo->threadNum, mainInfo->filename);
    free(mainInfo->backupPath);
    free(mainInfo);
    pthread_mutex_destroy(&threadNum_mutex);
}
