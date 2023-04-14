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
void* restoreFiles(void* args);

// threadNum incremented each time a new dir or file is backed up / restored
int threadNum = 0;
pthread_mutex_t threadNum_mutex;

short isRestore = 0;

typedef struct dirInfo{
    int threadNum;
    char* dirPath;
    char* dirBackupPath;
} dirInfo;

typedef struct fileInfo{
    int threadNum;
    char* filename;
    char* fileSrcPath;
    char* fileDestPath;
} fileInfo;

int openFile(char* filename, int flags, int perms){
    int fd;
    if(fd = open(filename, flags, perms), fd < 0){
        printf("ERROR: %s\n", strerror(errno));
        printf("problem opening file '%s'\nABORTING...\n", filename);
    }
    return fd;
}

void writeFile(fileInfo* fInfo){
    int src = openFile(fInfo->fileSrcPath, O_RDONLY, 0);
    int dest = openFile(fInfo->fileDestPath, O_WRONLY|O_CREAT, 0666);

    // int totalBytesWritten = writeFile(src, dest);
    int buffSize = 56, amtRead = 0;
    char buffer[buffSize];

    int totalBytesWritten = 0;
    while(amtRead = read(src, buffer, buffSize), amtRead > 0){
        totalBytesWritten += amtRead;
        write(dest, buffer, amtRead);
    }

    printf("[thread %d] Copied %d bytes from %s to %s\n", fInfo->threadNum, totalBytesWritten, fInfo->fileSrcPath, fInfo->fileDestPath);

    close(src);
    close(dest);
}

void* copyToBackupFile(void* args){
    fileInfo *fInfo = (fileInfo*)args;
    int src, dest;
    struct stat srcStat, destStat;

    int srcStatStatus = stat(fInfo->fileSrcPath, &srcStat);
    int destStatStatus = stat(fInfo->fileDestPath, &destStat);
    if(destStatStatus < 0){
        writeFile(fInfo);
        return NULL;
    }
   
    time_t srcTime, destTime;
    srcTime = srcStat.st_ctime;
    destTime = destStat.st_ctime;
    
    long diffTime = difftime(srcTime, destTime);
    
    if(diffTime > 0){
        printf("[thread %d] WARNING: Overwriting %s.bak\n", fInfo->threadNum, fInfo->filename);
    }
    else{
        printf("[thread %d] %s does not need backing up\n", fInfo->threadNum, fInfo->filename);
        return NULL;
    }

    writeFile(fInfo);

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
    newInfo->fileSrcPath = pathFromArray(filePath);

    char* fileBackupPath[] = {dirPath, "/.backup/", filename, ".bak", NULL};
    newInfo->fileDestPath = pathFromArray(fileBackupPath);

    return newInfo;
}

void followDir(char* dirPath, char* newDirName){
    char* rootDirPath[] = {dirPath, "/", newDirName, NULL};
    char* rootDir = pathFromArray(rootDirPath);
    pthread_mutex_lock(&threadNum_mutex);
    dirInfo* dtInfo = initDirInfo(rootDir);
    pthread_mutex_unlock(&threadNum_mutex);
    
    pthread_t t1;
    if(isRestore)
        pthread_create(&t1, NULL, &restoreFiles, dtInfo);
    else
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
    
    free(fInfo->fileDestPath);
    free(fInfo->fileSrcPath);
    free(fInfo);
}

fileInfo* initRestoreFileInfo(char* filename, char* dirPath){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    
    char* filePath[] = {dirPath, "/.backup/", filename, NULL};
    newInfo->fileSrcPath = pathFromArray(filePath);

    int i = 0, destLen = strlen(filename) - 4;
    char destPath[destLen];
    for(i; i < destLen; i++)
        destPath[i] = filename[i];
    destPath[i] = '\0';

    char* fileBackupPath[] = {dirPath, "/", destPath,  NULL};
    newInfo->fileDestPath = pathFromArray(fileBackupPath);

    return newInfo;
}

void* restoreFile(void* args){
    fileInfo *fInfo = (fileInfo*)args;
    int src, dest;
    struct stat srcStat, destStat;

    int srcStatStatus = stat(fInfo->fileSrcPath, &srcStat);
    int destStatStatus = stat(fInfo->fileDestPath, &destStat);
    if(destStatStatus < 0){
        writeFile(fInfo);
        return NULL;
    }
   
    time_t srcTime, destTime;
    srcTime = srcStat.st_ctime;
    destTime = destStat.st_ctime;
    
    long diffTime = difftime(destTime, srcTime);
    
    if(diffTime >= 0){
        int i = 0, len = strlen(fInfo->filename) - 4;
        char filename[len];
        for(i; i < len; i++)
            filename[i] = fInfo->filename[i];
        filename[i] = '\0';
        printf("[thread %d] %s is already the most current version\n", fInfo->threadNum, filename);
        return NULL;
    }

    writeFile(fInfo);
    return NULL;
}

void restoreBackups(dirInfo* dInfo){
    DIR* backupDirPath = opendir(dInfo->dirBackupPath);
    if(backupDirPath == NULL)
        return;

    struct dirent* file = readdir(backupDirPath);
    while (file = readdir(backupDirPath), file) {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            continue;
        
        pthread_t t1;
        fileInfo* fInfo = initRestoreFileInfo(file->d_name, dInfo->dirPath);
        pthread_create(&t1, NULL, &restoreFile, fInfo);
        pthread_join(t1, NULL);
        free(fInfo->fileDestPath);
        free(fInfo->fileSrcPath);
        free(fInfo);
    }
    
    closedir(backupDirPath);
}

void* restoreFiles(void* args) {
    dirInfo* dInfo = (dirInfo*)args;
    char* dirPath = dInfo->dirPath;
    
    DIR* dir = opendir(dirPath);
    if (dir == NULL)
        return NULL;

    struct dirent* file;
    file = readdir(dir);
    while (file = readdir(dir), file) {
        if(file->d_type == DT_DIR){
            if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
                continue;
         
            if(!strcmp(file->d_name, ".backup"))
                restoreBackups(dInfo);
            else
                followDir(dirPath, file->d_name);
        }
    }

    closedir(dir);
    return NULL;
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

    struct dirent* file = readdir(dir);
    while (file = readdir(dir), file) {
        if(file->d_type == DT_DIR){
            if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..") || !strcmp(file->d_name, ".backup"))
                continue;
            followDir(dirPath, file->d_name);
        }
        else if(file->d_type == DT_REG){
            backupFile(file->d_name, dirPath);
        }
    }

    closedir(dir);
    return NULL;
}

int main(int argc, char* argv[]){
    if(argc > 1){
        if(strcmp(argv[1], "-r") == 0)
            isRestore = 1;
        else{
            printf("Invalid arguments\nUsage: ./BackItUp [-r]\n");
            return 1;
        }
    }

    pthread_mutex_init(&threadNum_mutex, NULL);
    dirInfo* mainInfo = initDirInfo(".");

    pthread_t t1;
    if(isRestore)
        pthread_create(&t1, NULL, &restoreFiles, mainInfo);
    else
        pthread_create(&t1, NULL, &backupFiles, mainInfo);
    pthread_join(t1, NULL);

    free(mainInfo->dirBackupPath);
    free(mainInfo);
    pthread_mutex_destroy(&threadNum_mutex);
}
