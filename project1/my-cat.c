#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/*

Jared Diamond
Professor Bonamy
CS 460

A simple program that mimics the linux utility, cat. Files found on the cmd line
will be printed to stdout. I used read, write, and open because I wanted to
reacquaint myself with them. They might be useful in future assignments.

*/

int main(int argc, char *argv[])
{
    // no files found on the command line
    if(argc < 2)
        return 0;
    
    int fd;
    int buffSize = 56;
    char buffer[buffSize];
    int fileCount = 1;      // first file found at argv[1]
    
    // while files found in argv, print them to stdout
    while(fileCount < argc){
        if(fd = open(argv[fileCount], O_RDONLY), fd < 0){
            printf("my-cat: cannot open file\n");
            exit(1);
        }

        int amtRead = 0;
        while(amtRead = read(fd, buffer, buffSize), amtRead > 0)
            write(STDOUT_FILENO, buffer, amtRead);
        
        fileCount++;
        close(fd);
    }
   
    return 0;
}
