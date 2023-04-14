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
#include <time.h>

void* backupFiles(void* args);

// threadNum incremented each time a new dir or file is backed up / restored
int threadNum = 0;
pthread_mutex_t threadNum_mutex;

// -r option passed when invoking program
short int isRestore;


typedef struct dirInfo{
    int threadNum;
    char* dirPath;
    char* dirBackupPath;
} dirInfo;

typedef struct fileInfo{
    int threadNum;
    char* filename;
    char* filePath;
    char* fileBackupPath;
} fileInfo;

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

void* restoreFile(void* args){

    return NULL;
}

void* copyToBackupFile(void* args){
    fileInfo *fInfo = (fileInfo*)args;
    int src, dest;
    struct stat srcStat, destStat;

    src = openFile(fInfo->filePath, O_RDONLY, 0);
    dest = openFile(fInfo->fileBackupPath, O_WRONLY|O_CREAT, 0666);

    stat(fInfo->filePath, &srcStat);
    stat(fInfo->fileBackupPath, &destStat);
   
    time_t srcTime, destTime;
    srcTime = srcStat.st_mtime;
    destTime = destStat.st_mtime;

    if(srcTime > destTime){
        printf("[thread %d] WARNING: Overwriting %s.bak\n", fInfo->threadNum, fInfo->filename);
    }
    else{
        printf("[thread %d] %s does not need backing up\n", fInfo->threadNum, fInfo->filename);
        return NULL;
    }

    int totalBytesWritten = writeFile(src, dest);

    printf("[thread %d] Copied %d bytes from %s to %s.bak\n", fInfo->threadNum, totalBytesWritten, fInfo->filename, fInfo->filename);

    close(src);
    close(dest);

    return NULL;
}

char* pathFromArray(char* pathArr[]){
    char* newPath = calloc(1, sizeof(char));
    int pathLen = 0;
    
    for(int i = 0; pathArr[i] != NULL; i++){
        pathLen += strlen(pathArr[i]);
        newPath = realloc(newPath,  (pathLen + 1) * sizeof(char));
        strcat(newPath, pathArr[i]);
    }
    return newPath;
}

dirInfo* initDirInfo(char* dirPath){
    dirInfo* newInfo = malloc(sizeof(dirInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->dirPath = dirPath;

    char* dirBackupPath[] = {dirPath, "/.backup", NULL};
    newInfo->dirBackupPath = pathFromArray(dirBackupPath);
    return newInfo;
}

fileInfo* initFileInfo(char* filename, char* dirPath){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    
    char* filePath[] = {dirPath, "/", filename, NULL};
    newInfo->filePath = pathFromArray(filePath);

    char* fileBackupPath[] = {dirPath, "/.backup/", filename, ".bak", NULL};
    newInfo->fileBackupPath = pathFromArray(fileBackupPath);

    return newInfo;
}

void backupDir(char* dirPath, char* newDirName){
    char* rootDirPath[] = {dirPath, "/", newDirName, NULL};
    char* rootDir = pathFromArray(rootDirPath);
    pthread_mutex_lock(&threadNum_mutex);
    dirInfo* dtInfo = initDirInfo(rootDir);
    pthread_mutex_unlock(&threadNum_mutex);
    
    pthread_t t1;
    pthread_create(&t1, NULL, &backupFiles, dtInfo);
    pthread_join(t1, NULL);
    
    free(rootDir);
    free(dtInfo->dirBackupPath);
    free(dtInfo);
}

void backupFile(char* filename, char* dirPath){
    pthread_mutex_lock(&threadNum_mutex);
    fileInfo* fInfo = initFileInfo(filename, dirPath);
    pthread_mutex_unlock(&threadNum_mutex);
    
    pthread_t t1;
    pthread_create(&t1, NULL, &copyToBackupFile, fInfo);
    pthread_join(t1, NULL);
    
    free(fInfo->fileBackupPath);
    free(fInfo->filePath);
    free(fInfo);
}

void* backupFiles(void* args) {
    dirInfo* dInfo = (dirInfo*)args;
    char* dirPath = dInfo->dirPath;
    
    DIR* dir = opendir(dirPath);
    if (dir == NULL)
        return NULL;

    char* dirBackupPath = dInfo->dirBackupPath;
    if(mkdir(dirBackupPath, 0777) < 0 && errno != EEXIST)
        printf("mkdir error: %s\n", strerror(errno));

    DIR* backupDirPath;
    backupDirPath = opendir(dirBackupPath);
    if(backupDirPath == NULL)
        return NULL;

    struct dirent* file;
    file = readdir(dir);
    while (file = readdir(dir), file) {
        if(file->d_type == DT_DIR){
            if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..") || !strcmp(file->d_name, ".backup"))
                continue;
            backupDir(dirPath, file->d_name);
        }
        else if(file->d_type == DT_REG){
            backupFile(file->d_name, dirPath);
        }
    }

    closedir(dir);
    closedir(backupDirPath);
    return NULL;
}

int main(int argc, char* argv[]){
    pthread_mutex_init(&threadNum_mutex, NULL);
    dirInfo* mainInfo = initDirInfo(".");

    pthread_t t1;
    pthread_create(&t1, NULL, &backupFiles, mainInfo);
    pthread_join(t1, NULL);

    // listFiles(mainInfo->filename);
    free(mainInfo->dirBackupPath);
    free(mainInfo);
    pthread_mutex_destroy(&threadNum_mutex);
}
