#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void printFile(FILE* fp);

/*

Jared Diamond
Professor Bonamy
CS 460

A simple program that mimics the linux utility, uniq. Will print out files
found on the command line, ignoring lines that are adjacent duplicates. If
no files are present on the cmd line, my-uniq will use stdin

*/

int main(int argc, char *argv[])
{
    // if files found on cmd line: loop through them and print them out
    // else: print out stdin
    if(argc > 1){
        int fileCount = 1;  // first file found at argv[1]
        while(fileCount < argc){
            FILE* fp = fopen(argv[fileCount], "r");
            if(!fp){
                printf("my-uniq: cannot open file\n");
                exit(1);
            }

            printFile(fp); 
            fclose(fp);
            fileCount++;
        }
    }
    else
        printFile(stdin);

    return 0;
}

// prints given file (fp) to stdout, ignoring adjacent duplicate lines
void printFile(FILE* fp){
    size_t len = 0;
    char* line = NULL;
    ssize_t nRead;
    // last seen line
    char* lastLine = NULL;
    
    // while the file contains lines, compare them to the last seen line
    // and print them out if they are not the same
    while(nRead = getline(&line, &len, fp), nRead > 0){
        // current line is not the same as lastLine, print it out and update lastLine
        if(lastLine == NULL || strcmp(lastLine, line)){
            printf("%s", line);
            lastLine = realloc(lastLine, strlen(line) + 1);
            strcpy(lastLine, line);
        }
        // current line is the same as lastLine, do nothing
    }

    free(lastLine);
    free(line);
}
