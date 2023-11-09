#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void verifyInput(int argc, char *argv[]){
    if(argc != 2){
        exit(0);
    }
    if(strcmp(argv[1] + strlen(argv[1]) - 4, ".bmp") != 0){
        exit(0);
    }
}

void writePhotoDimensions(char *fileName){
    struct stat status;
    stat(fileName, &status);
    int fd=open(fileName, O_RDONLY);
    lseek(fd, 18, SEEK_CUR);
    int width = 0;
    int heigth = 0;
    read(fd, &width, sizeof(int));
    read(fd, &heigth, sizeof(int));
    printf("inaltime: %d\n",heigth);
    printf("lungime: %d\n",width);
    close(fd);
}

int main(int argc, char *argv[]){
  
    verifyInput(argc, argv);

    printf("nume fisier: %s\n",argv[1]);

    writePhotoDimensions(argv[1]);

    return 0;
}
