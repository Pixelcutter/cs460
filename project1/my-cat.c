/**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    // no files specified on the command line
    if(argc < 2)
        return 0;
    
    int fd;
    int buffSize = 56;
    char buffer[buffSize];
    int fileCount = 1;
    
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
