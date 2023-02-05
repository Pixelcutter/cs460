#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void printFile(FILE*fp, char* oldStr, char* newStr);
    
/*

Jared Diamond
Professor Bonamy
CS 460

A simple program that mimics the linux utility, sed. Will print out files
found on the command line, replacing the first instance of argv[1] with argv[2]. 
Filenames should follow argv[2]. If no files are found on the cmd line, stdin
will be used. 

THIS PROGRAM WILL NOT RECOGNIZE REGEX

*/

int main(int argc, char *argv[])
{   
    // not enough arguments found on cmd line: print usage info and exit
    if(argc < 3){
        printf("my-sed: find term replace term [file ...]\n");
        exit(1);
    }    
    
    // if files found on cmd line: loop through them and print them out 
    // else: print out stdin
    if(argc > 3){
        int fileCount = 3; // first file found at argv[3], after find and replace args
        while(fileCount < argc){
            FILE* fp = fopen(argv[fileCount], "r");
            if(!fp){
                printf("my-sed: cannot open file\n");
                exit(1);
            }

            printFile(fp, argv[1], argv[2]);
            fclose(fp);
            fileCount++;
        }
    }
    else
        printFile(stdin, argv[1], argv[2]);

    return 0;    
}

/*
    Constructs and returns a new string based on a source string. The first instance
    of oldStr found in the source string is replaced with newStr in the constructed
    string

    startStr:   strstr() pointer to oldStr found in source string
    oldStr:     string to be replaced in source
    newStr:     string that replaces oldStr in constructed string
*/
char* replace(char* source, char* startStr, char* oldStr, char* newStr){
    // lengths of given strings and their combinations
    int srcLen = strlen(source);
    int startLen = strlen(startStr);
    int newLen = strlen(newStr);
    int oldLen = strlen(oldStr);
    int lenDiff = newLen - oldLen;
    int totalLen = srcLen + lenDiff + 1; // +1 for null terminator
    int startI = srcLen - startLen; // the index where oldStr begins in source

    // new string constructed
    char* catStr = calloc(sizeof(char) * totalLen, sizeof(char));
    memcpy(catStr, source, startI);
    memcpy(catStr + startI, newStr, newLen);
    memcpy(catStr + startI + newLen, startStr + oldLen, startLen - oldLen);

    return catStr;
}

// prints given file (fp) to stdout. The first found instance of oldStr
// will be replaced (non-destructive) with newStr
void printFile(FILE*fp, char* oldStr, char* newStr){
    char* startStr;
    size_t len = 0;
    char* line = NULL;
    ssize_t nRead;

    // flag that indicates whether or not the term has been found and replaced
    int targetFound = 0;

    // while the file contains lines and nothing has been replaced, search for
    // oldStr, replace if found, and print line to stdout
    while(nRead = getline(&line, &len, fp), nRead > 0){
        if(!targetFound){
            // oldStr found in source line, replace it and update found flag
            if((startStr = strstr(line, oldStr))){
                char* tmpLine = replace(line, startStr, oldStr, newStr);
                printf("%s", tmpLine);
                targetFound = 1;
                free(tmpLine);
                continue;
            }
        }
        printf("%s", line);
    }
    
    free(line);
}

