/*
    Jared Diamond
    CS 460
    Professor Bonamy

    Description: With no options, this program will recursively back up files found in a directory tree
    to a .backup directory within each directory. Passing an -r option when invoking this program will
    tell the program to restore backup files to their original name/location. Each file is backed up or
    restored with a seperate thread. 
*/
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

// used by threads that are traversing a directory tree
typedef struct dirInfo{
    char* dirPath;          // path to directory
    char* dirBackupPath;    // .backup path for directory
} dirInfo;

typedef struct fileInfo{
    int threadNum;
    char* filename;         
    char* fileSrcPath;      // source path of file, base file in backup mode, .bak file in restore mode
    char* fileDestPath;     // destination path of file, .bak file in restore mode, base file in restore mode
} fileInfo;

// linked list node that holds fileInfo structs
// for end of program printout
typedef struct node{
    fileInfo* fInfo;
    long bytesWritten;
    struct node* next;
} NODE;


// threadNum incremented each time a new dir or file is backed up / restored
int threadNum = 0;
pthread_mutex_t threadNum_mutex;  // mutex for threadNum

// flag indicating whether or not program is in restore mode
short isRestore = 0;

// head of node linked list
NODE* rootNode;
pthread_mutex_t rootNode_mutex; // mutex that controls access to LL

/****************************** Utility functions *******************************/

// initializes a new LL node
NODE* initNode(fileInfo* fInfo, int bytesWritten){
    NODE* newNode = malloc(sizeof(NODE));
    newNode->fInfo = fInfo;
    newNode->bytesWritten = bytesWritten;
    newNode->next = NULL;

    return newNode;
}

// adds a new node to an existing linked list
NODE* addNode(NODE* root, NODE* newNode){
    NODE* tmp = root;
    
    while(tmp->next)
        tmp = tmp->next;
    tmp->next = newNode;

    return root;
}

// utility function that concatenates an array of strings together
// used to construct file path strings
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

// function that abstracts away the boilerplate involved with opening a file
// and checking for errors
int openFile(char* filename, int flags, int perms){
    int fd;
    if(fd = open(filename, flags, perms), fd < 0){
        printf("ERROR: %s\n", strerror(errno));
        printf("problem opening file '%s'\n", filename);
        exit(1);
    }
    return fd;
}

// simple function that writes the contents of one file to another file
void writeFile(fileInfo* fInfo){
    int src = openFile(fInfo->fileSrcPath, O_RDONLY, 0);
    int dest = openFile(fInfo->fileDestPath, O_WRONLY|O_CREAT, 0666);

    int buffSize = 56, amtRead = 0;
    char buffer[buffSize];

    int bytesWritten = 0;
    while(amtRead = read(src, buffer, buffSize), amtRead > 0){
        bytesWritten += amtRead;
        write(dest, buffer, amtRead);
    }

    // new node created and added to rootNode LL
    NODE* newNode = initNode(fInfo, bytesWritten);
    pthread_mutex_lock(&rootNode_mutex);
    rootNode = addNode(rootNode, newNode);
    pthread_mutex_unlock(&rootNode_mutex);

    close(src);
    close(dest);
}

// prints final program stats to stdout
void finalPrint(NODE* root){
    long totalBytesWritten = 0;
    int totalFilesAffected = 0;
    NODE* tmp = root;
    NODE* prev = tmp;

    // while LL has nodes: print each nodes threadNum, bytesWritten, 
    // file source path, and file destination path
    while(tmp = tmp->next, tmp){
        free(prev);
        printf("[thread %d] Copied %ld bytes from %s to %s\n", 
                tmp->fInfo->threadNum, 
                tmp->bytesWritten, 
                tmp->fInfo->fileSrcPath, 
                tmp->fInfo->fileDestPath);

        totalFilesAffected++;
        totalBytesWritten += tmp->bytesWritten;    
        
        free(tmp->fInfo->fileDestPath);
        free(tmp->fInfo->fileSrcPath);
        free(tmp->fInfo);
        prev = tmp;
    }
    free(prev);

    // print final totals only if LL was not empty
    if(totalFilesAffected > 0)
        printf("Successfully copied %d files (%ld bytes)\n", 
                totalFilesAffected, 
                totalBytesWritten);
}


/************************** Directory related functions ***************************/

// initializes a new dirInfo struct and then returns it
dirInfo* initDirInfo(char* dirPath){
    dirInfo* newInfo = malloc(sizeof(dirInfo));
    newInfo->dirPath = dirPath;

    char* dirBackupPath[] = {dirPath, "/.backup", NULL};
    newInfo->dirBackupPath = pathFromArray(dirBackupPath);
    return newInfo;
}

// used to create a new thread that will traverse a newly constructed
// directory path
void followDir(char* dirPath, char* newDirName){
    char* rootDirPath[] = {dirPath, "/", newDirName, NULL};
    char* rootDir = pathFromArray(rootDirPath);
    dirInfo* dtInfo = initDirInfo(rootDir);
    
    pthread_t t1;
    // if in restore mode: send thread to restoreFiles function
    // else: send thread to backupFiles function
    if(isRestore)
        pthread_create(&t1, NULL, &restoreFiles, dtInfo);
    else
        pthread_create(&t1, NULL, &backupFiles, dtInfo);
    pthread_join(t1, NULL);
    
    // thread has been joined and the directory has been 
    // successfully traversed. Time to free the dirInfo
    free(rootDir);
    free(dtInfo->dirBackupPath);
    free(dtInfo);
}

/************************ Backup mode related functions *************************/

// initializes a new fileInfo
// used when backing up a file from file -> file.bak
fileInfo* initBackupFileInfo(char* filename, char* dirPath){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    
    // source path = source_dir/filename
    char* filePath[] = {dirPath, "/", filename, NULL};
    newInfo->fileSrcPath = pathFromArray(filePath);

    // destination path = source_dir/.backup/filename.bak
    char* fileBackupPath[] = {dirPath, "/.backup/", filename, ".bak", NULL};
    newInfo->fileDestPath = pathFromArray(fileBackupPath);

    return newInfo;
}

// backs up a file from filename to filename.bak if source file's
// last modification time is newer than backup files last modfication time
void* copyToBackupFile(void* args){
    fileInfo *fInfo = (fileInfo*)args;
    struct stat srcStat, destStat;

    printf("[thread %d] Backing up %s\n", fInfo->threadNum, fInfo->filename);

    stat(fInfo->fileSrcPath, &srcStat);
    int destStatStatus = stat(fInfo->fileDestPath, &destStat);
    // if destination file does not exist: create it
    if(destStatStatus < 0){
        writeFile(fInfo);
        return NULL;
    }
   
    // last modification times of source and destination files
    time_t srcTime, destTime;
    srcTime = srcStat.st_mtime;
    destTime = destStat.st_mtime;
    long diffTime = difftime(srcTime, destTime); // difference between last modified times
    
    // if original file's last modified time > backup file modified time:
    // warn user of backup overwrite and then overwrite backup
    // else: inform user that the original file does not need to be backed up and return
    if(diffTime > 0){
        printf("[thread %d] WARNING: Overwriting %s.bak\n", 
                fInfo->threadNum, 
                fInfo->filename);

        writeFile(fInfo);
    }
    else{
        printf("[thread %d] %s does not need backing up\n", 
                fInfo->threadNum, 
                fInfo->filename);

        free(fInfo->fileSrcPath);
        free(fInfo->fileDestPath);
        free(fInfo);
        return NULL;
    }

    return NULL;
}

// go-between function that initializes a new fileInfo struct
// and sends a thread off to copy the given file
void backupFile(char* filename, char* dirPath){
    pthread_mutex_lock(&threadNum_mutex);
    fileInfo* fInfo = initBackupFileInfo(filename, dirPath);
    pthread_mutex_unlock(&threadNum_mutex);
    
    pthread_t t1;
    pthread_create(&t1, NULL, &copyToBackupFile, fInfo);
    pthread_join(t1, NULL);
}

// recursive function (followDir() will call this function again) that traverses
// a directory tree and backs up regular files to .backup directories. Each directory
// traversed will have its own .backup directory (created if it does not exist)
void* backupFiles(void* args) {
    dirInfo* dInfo = (dirInfo*)args;
    char* dirPath = dInfo->dirPath;
    
    DIR* dir = opendir(dirPath);
    if (dir == NULL)
        return NULL;

    // dirPath/.backup directory created if it does not already exist    
    char* dirBackupPath = dInfo->dirBackupPath;
    // if error from mkdir != directory already exists: report error and exit
    if(mkdir(dirBackupPath, 0777) < 0 && errno != EEXIST){
        printf("mkdir error: %s\n", strerror(errno));
        exit(1);
    }

    // while files/directories found in directory, loop through and back them up
    struct dirent* file = readdir(dir);
    while (file = readdir(dir), file) {
        // if file is a directory && and it's not '.', '..', or '.backup': follow it
        // else if file is a regular file: back it up
        if(file->d_type == DT_DIR){
            if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..") || !strcmp(file->d_name, ".backup"))
                continue;
            // go deeper in directory tree
            followDir(dirPath, file->d_name);
        }
        else if(file->d_type == DT_REG){
            // regular file found, back it up
            backupFile(file->d_name, dirPath);
        }
    }

    closedir(dir);
    return NULL;
}

/************************ Restore mode related functions *************************/

// initializes a new fileInfo
// used when restoring a file from file.bak -> file
fileInfo* initRestoreFileInfo(char* filename, char* dirPath){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    newInfo->threadNum = ++threadNum;
    newInfo->filename = filename;
    
    // source path = source_dir/.backup/filename.bak
    char* filePath[] = {dirPath, "/.backup/", filename, NULL};
    newInfo->fileSrcPath = pathFromArray(filePath);

    // stripping .bak from filename
    int i, destLen = strlen(filename) - 4; // len(.bak) = 4
    char destPath[destLen];
    for(i = 0; i < destLen; i++)
        destPath[i] = filename[i];
    destPath[i] = '\0';

    // destination path = source_dir/filename
    char* fileBackupPath[] = {dirPath, "/", destPath,  NULL};
    newInfo->fileDestPath = pathFromArray(fileBackupPath);

    return newInfo;
}

// restores a file from directory_path/.backup/filename.bak to directory_path/filename if original file
// is not newer than the backup
void* restoreFile(void* args){
    fileInfo *fInfo = (fileInfo*)args;
    struct stat srcStat, destStat;

    // stripping '.bak'
    int i, len = strlen(fInfo->filename) - 4; // len(.bak) = 4
    char filename[len];
    for(i = 0; i < len; i++)
        filename[i] = fInfo->filename[i];
    filename[i] = '\0';
    
    printf("[thread %d] Restoring %s\n", fInfo->threadNum, filename);

    stat(fInfo->fileSrcPath, &srcStat);
    int destStatStatus = stat(fInfo->fileDestPath, &destStat);
    // if destination file does not exist: create it
    if(destStatStatus < 0){
        writeFile(fInfo);
        return NULL;
    }
   
    // last modified times of source and destination files
    time_t srcTime, destTime;
    srcTime = srcStat.st_mtime;
    destTime = destStat.st_mtime;
    long diffTime = difftime(destTime, srcTime); // difference between modified times
    
    // if original file's last modified time > backup file modified time:
    // free fileInfo and return without restoring
    if(diffTime >= 0){
        printf("[thread %d] %s is already the most current version\n", 
                fInfo->threadNum, 
                filename);

        free(fInfo->fileSrcPath);
        free(fInfo->fileDestPath);
        free(fInfo);
        return NULL;
    }

    // restore original file from backup
    writeFile(fInfo);
    return NULL;
}

// loops through a given directory (path found in dInfo) and restores .bak
// files to their original file name/location
void restoreBackups(dirInfo* dInfo){
    DIR* backupDirPath = opendir(dInfo->dirBackupPath);
    if(backupDirPath == NULL)
        return;

    // while files present in directory, loop through and restore them
    struct dirent* file = readdir(backupDirPath);
    while (file = readdir(backupDirPath), file) {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            continue;
        
        pthread_t t1;
        fileInfo* fInfo = initRestoreFileInfo(file->d_name, dInfo->dirPath);
        pthread_create(&t1, NULL, &restoreFile, fInfo);
        pthread_join(t1, NULL);
    }
    
    closedir(backupDirPath);
}

// recursive function (followDir() will call this function again) that traverses
// a directory tree and restores .bak files found in .backup directories. 
void* restoreFiles(void* args) {
    dirInfo* dInfo = (dirInfo*)args;
    char* dirPath = dInfo->dirPath;
    
    DIR* dir = opendir(dirPath);
    if (dir == NULL)
        return NULL;

    struct dirent* file;
    file = readdir(dir);
    while (file = readdir(dir), file) {
        // if file is a directory && and it's not '.', '..': follow or back it up
        // else if file is a regular file: back it up// while files present in directory, loop through and restore them
        if(file->d_type == DT_DIR){
            if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
                continue;

            // if directory is .backup: restore .bak files
            // else: follow it
            if(!strcmp(file->d_name, ".backup"))
                restoreBackups(dInfo);
            else
                followDir(dirPath, file->d_name);
        }
    }

    closedir(dir);
    return NULL;
}

/********************************* Main function **********************************/

int main(int argc, char* argv[]){
    // if arguments found on cmd line and the first one == '-r': switch to restore mode
    // else: inform user of correct usage and exit
    if(argc > 1){
        if(strcmp(argv[1], "-r") == 0)
            isRestore = 1;
        else{
            printf("Invalid arguments\nUsage: ./BackItUp [-r]\n");
            exit(1);
        }
    }

    // LL initialized
    rootNode = initNode(NULL, 0);

    pthread_mutex_init(&threadNum_mutex, NULL);
    pthread_mutex_init(&rootNode_mutex, NULL);

    // main directory initialized
    dirInfo* mainInfo = initDirInfo(".");

    // if in restore mode: restore files
    // else: back up files
    pthread_t t1;
    if(isRestore)
        pthread_create(&t1, NULL, &restoreFiles, mainInfo);
    else
        pthread_create(&t1, NULL, &backupFiles, mainInfo);
    pthread_join(t1, NULL);

    // print final stats
    finalPrint(rootNode);
    
    free(mainInfo->dirBackupPath);
    free(mainInfo);
    pthread_mutex_destroy(&threadNum_mutex);
    pthread_mutex_destroy(&rootNode_mutex);
}