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

typedef struct dirInfo{
    int threadNum;
    char* rootDir;
    char* backupDirPath;
} dirInfo;

typedef struct fileInfo{
    int threadNum;
    char* filename;
    char* filePath;
    char* fileBackupPath;
} fileInfo;

char* appendBackup(char* filePath){
    int len = strlen(filePath);
    char* newStr = calloc(len + 9, sizeof(char)); // len('/.backup\0') = 9
    strcat(newStr, filePath);
    strcat(newStr, "/.backup");

    return newStr;
}

char* appendBak(char* filename){
    char* newFilename = calloc(strlen(filename) + 5, sizeof(char));
    strcat(newFilename, filename);
    strcat(newFilename, ".bak");
    return newFilename;
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
    fileInfo *fInfo = (fileInfo*)args;
    char* backupFilePath = appendBak(fInfo->filename);
    int src, dest;

    src = openFile(fInfo->filePath, O_RDONLY, 0);
    dest = openFile(fInfo->fileBackupPath, O_WRONLY|O_CREAT, 0666);
    int totalBytesWritten = writeFile(src, dest);

    printf("[thread %d] Copied %d bytes from %s to %s.bak\n", fInfo->threadNum, totalBytesWritten, fInfo->filename, fInfo->filename);

    close(src);
    close(dest);
    free(fInfo->filePath);
    free(fInfo->fileBackupPath);    

    return NULL;
}

char* buildPath(char* rootDir, char* dName){
    char* path = calloc(strlen(rootDir) + strlen(dName) + 2, sizeof(char));
    strcat(path, rootDir);
    strcat(path, "/");
    strcat(path, dName);

    return path;
}

threadInfo* initThreadInfo(char* filename, char* rootDir){
    threadInfo* newInfo = malloc(sizeof(threadInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    newInfo->rootDir = rootDir;
    newInfo->backupPath = appendBackup(rootDir);
    return newInfo;
}

fileInfo* initFileInfo(char* filename, char* dirname){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    newInfo->filePath = buildPath(dirname, filename);
    newInfo->fileBackupPath = buildPath(appendBackup(dirname), appendBak(filename));
    return newInfo;
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
    if(mkdir(backupPath, 0777) < 0 && errno != EEXIST)
        printf("mkdir error: %s\n", strerror(errno));

    DIR* backupDir;
    backupDir = opendir(backupPath);
    // printf("backupDir: %s\n", backupPath);
    if(backupDir == NULL)
        return NULL;

    struct dirent* entity;
    entity = readdir(dir);
    while (entity = readdir(dir), entity) {
        // printf("%hhd %s/%s\n", entity->d_type, dirname, entity->d_name);
        if(entity->d_type == DT_DIR){
            if(!strcmp(entity->d_name, ".") || !strcmp(entity->d_name, "..") || !strcmp(entity->d_name, ".backup"))
                continue;

            // printf("d_name: %s\n", entity->d_name);
            char* rootDir = buildPath(dirname, entity->d_name);
            pthread_mutex_lock(&threadNum_mutex);
            dtInfo = initThreadInfo(NULL, rootDir);
            pthread_mutex_unlock(&threadNum_mutex);
            pthread_t thread;
            pthread_create(&thread, NULL, &listFiles, dtInfo);
            pthread_join(thread, NULL);
            
            free(rootDir);
            free(dtInfo->backupPath);
            free(dtInfo);
        }
        else if(entity->d_type == DT_REG){
            pthread_t t1;
            pthread_mutex_lock(&threadNum_mutex);
            fileInfo* fInfo = initFileInfo(entity->d_name, dirname);
            pthread_mutex_unlock(&threadNum_mutex);
            // printf("filename: %s | rootdir: %s\n", fInfo->filename, dirname);
            // printf("regular path: %s\n", fInfo->filePath);
            // printf("backup path: %s\n", fInfo->fileBackupPath);
            pthread_create(&t1, NULL, &copyFile, fInfo);
            pthread_join(t1, NULL);
        }
    }

    closedir(dir);
    closedir(backupDir);
}

int main(int argc, char* argv[]){
    pthread_mutex_init(&threadNum_mutex, NULL);
    threadInfo* mainInfo = initThreadInfo(NULL, ".");

    pthread_t t1;
    pthread_create(&t1, NULL, &listFiles, mainInfo);
    pthread_join(t1, NULL);

    // listFiles(mainInfo->filename);
    free(mainInfo->backupPath);
    free(mainInfo);
    pthread_mutex_destroy(&threadNum_mutex);
}
