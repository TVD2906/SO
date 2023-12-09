#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s \n", argv[0]);
    return 1;
  }

  char *inputFileName = argv[1];
  char *outputFileName = argv[2];
  char character = argv[3][0];

  int inputFd = open(inputFileName, O_RDONLY);
  if (inputFd == -1) {
    perror("Error opening input file");
    return 1;
  }

  int outputFd = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (outputFd == -1) {
    perror("Error opening output file");
    close(inputFd);
    return 1;
  }

  struct stat fileInfo;
  if (fstat(inputFd, &fileInfo) == -1) {
    perror("Error getting file information");
    close(inputFd);
    close(outputFd);
    return 1;
  }
  int lowercaseCount = 0;
  int uppercaseCount = 0;
  int digitCount = 0;
  int charCount = 0;

  char buffer[1];
  while (read(inputFd, buffer, 1) > 0) {
    if (islower(buffer[0])) {
      lowercaseCount++;
    } else if (isupper(buffer[0])) {
      uppercaseCount++;
    } else if (isdigit(buffer[0])) {
      digitCount++;
    }
    if (buffer[0] == character) {
      charCount++;
    }
  }

  char output[256];
  sprintf(output, "numar litere mici: %d\nnumar litere mari: %d\nnumar cifre: %d\nnumar aparitii caracter: %d\ndimensiune fisier: %ld\n",
	  lowercaseCount, uppercaseCount, digitCount, charCount, fileInfo.st_size);
  if (write(outputFd, output, strlen(output)) == -1) {
    perror("Error writing to output file");
  }
  close(inputFd);
  close(outputFd);

  return 0;
}
