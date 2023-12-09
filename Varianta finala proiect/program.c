#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define MAX_PATH_LENGTH 256
#define HEADER_SIZE 54
#define IMAGE_SIZES 18
#define MAX_BUFFER_SIZE 4096

int totalLines = 0;

void rights(struct stat fileInfo, char *user, char *group, char *other) {
    strcat(user, (fileInfo.st_mode & S_IRUSR) ? "R" : "-");
    strcat(user, (fileInfo.st_mode & S_IWUSR) ? "W" : "-");
    strcat(user, (fileInfo.st_mode & S_IXUSR) ? "X" : "-");

    strcat(group, (fileInfo.st_mode & S_IRGRP) ? "R" : "-");
    strcat(group, (fileInfo.st_mode & S_IWGRP) ? "W" : "-");
    strcat(group, (fileInfo.st_mode & S_IXGRP) ? "X" : "-");

    strcat(other, (fileInfo.st_mode & S_IROTH) ? "R" : "-");
    strcat(other, (fileInfo.st_mode & S_IWOTH) ? "W" : "-");
    strcat(other, (fileInfo.st_mode & S_IXOTH) ? "X" : "-");
}

void removeExtension(const char *fileName) {
    char *lastDot = strrchr(fileName, '.');
    if (lastDot != NULL) {
        *lastDot = '\0';
    }
}

int isRegularFile(const char *fileName) {
    struct stat info;

    if (lstat(fileName, &info) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    return S_ISREG(info.st_mode);
}

void processImage(const char *inputFilePath, const char *outputDirectory, const char *filename) {
    int file = open(inputFilePath, O_RDWR);
    if (file == -1) {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    struct stat fileInfo;

    if (stat(inputFilePath, &fileInfo) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    if (lseek(file, IMAGE_SIZES, SEEK_SET) == -1) {
        perror("Error setting cursor\n");
        close(file);
        exit(EXIT_FAILURE);
    }

    int height = 0, width = 0;
    read(file, &width, 4);
    read(file, &height, 4);
    char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
    char date[20];
    char buffer[MAX_BUFFER_SIZE] = "";
    strftime(date, sizeof(date), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
    rights(fileInfo, userRights, groupRights, otherRights);

    fflush(NULL);

    removeExtension(filename);
    snprintf(buffer, sizeof(buffer), "File name: %s\n"
                                     "Height: %d\n"
                                     "Width: %d\n"
                                     "Size: %ld\n"
                                     "User ID: %d\n"
                                     "Last modification time: %s\n"
                                     "Link count: %ld\n"
                                     "User rights: %s\n"
                                     "Group rights: %s\n"
                                     "Other rights: %s\n\n",
             filename, height, width, fileInfo.st_size, fileInfo.st_uid, date, fileInfo.st_nlink, userRights, groupRights, otherRights);

    char statFileName[MAX_BUFFER_SIZE] = "";
    snprintf(statFileName, sizeof(statFileName), "%s/%s_statistics.txt", outputDirectory, filename);
    int fileDescriptorOut = open(statFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fileDescriptorOut == -1) {
        perror("Error opening output file\n");
        exit(-1);
    }

    if (strcmp(buffer, "") != 0) {
        if (write(fileDescriptorOut, buffer, strlen(buffer)) == -1) {
            perror("Error writing to output file\n");
            close(file);
            close(fileDescriptorOut);
            exit(EXIT_FAILURE);
        }
    }

    close(file);
    close(fileDescriptorOut);
}

void grayscaleImage(const char *inputFilePath) {
    int file = open(inputFilePath, O_RDWR);
    if (file == -1) {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    if (lseek(file, 0, SEEK_SET) == -1) {
        perror("Error resetting cursor to the beginning of the file\n");
        close(file);
        exit(EXIT_FAILURE);
    }

    if (lseek(file, HEADER_SIZE, SEEK_SET) == -1) {
        perror("Error jumping over header\n");
        close(file);
        exit(EXIT_FAILURE);
    }

    unsigned char pixel[3];
    while (read(file, pixel, 3) == 3) {
        unsigned char grayscalePixel = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);
        lseek(file, -3, SEEK_CUR);
        write(file, &grayscalePixel, 1);
        write(file, &grayscalePixel, 1);
        write(file, &grayscalePixel, 1);
    }

    close(file);

    exit(EXIT_SUCCESS);
}

void processFile(const char *inputFilePath, const char *outputDirectory, const char *filename) {

    struct stat fileInfo;

    if (lstat(inputFilePath, &fileInfo) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE] = "";

    if (S_ISREG(fileInfo.st_mode)) {
        strcpy(buffer, "");
        char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
        char date[20];
        strftime(date, sizeof(date), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
        rights(fileInfo, userRights, groupRights, otherRights);

        removeExtension(filename);
        snprintf(buffer, sizeof(buffer), "File name: %s\n"
                                         "Size: %ld\n"
                                         "User ID: %d\n"
                                         "Last modification time: %s\n"
                                         "Link count: %ld\n"
                                         "User rights: %s\n"
                                         "Group rights: %s\n"
                                         "Other rights: %s\n\n",
                 filename, fileInfo.st_size, fileInfo.st_uid, date, fileInfo.st_nlink, userRights, groupRights, otherRights);

        char statFileName[MAX_BUFFER_SIZE] = "";
        snprintf(statFileName, sizeof(statFileName), "%s/%s_statistics.txt", outputDirectory, filename);
        int fileDescriptorOut = open(statFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fileDescriptorOut == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if (strcmp(buffer, "") != 0) {
            if (write(fileDescriptorOut, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(fileDescriptorOut);
                exit(EXIT_FAILURE);
            }
        }

        close(fileDescriptorOut);
    } else if (S_ISDIR(fileInfo.st_mode)) {
        strcpy(buffer, "");
        char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
        rights(fileInfo, userRights, groupRights, otherRights);

        removeExtension(filename);
        snprintf(buffer, sizeof(buffer), "Directory name: %s\n"
                                         "User ID: %d\n"
                                         "User rights: %s\n"
                                         "Group rights: %s\n"
                                         "Other rights: %s\n\n",
                 filename, fileInfo.st_uid, userRights, groupRights, otherRights);

        char statFileName[MAX_BUFFER_SIZE] = "";
        snprintf(statFileName, sizeof(statFileName), "%s/%s_statistics.txt", outputDirectory, filename);
        int fileDescriptorOut = open(statFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fileDescriptorOut == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if (strcmp(buffer, "") != 0) {
            if (write(fileDescriptorOut, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(fileDescriptorOut);
                exit(EXIT_FAILURE);
            }
        }

        close(fileDescriptorOut);
    } else if (S_ISLNK(fileInfo.st_mode)) {
        strcpy(buffer, "");
        char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
        rights(fileInfo, userRights, groupRights, otherRights);
        struct stat targetFileInfo;
        if (stat(inputFilePath, &targetFileInfo) == -1) {
            perror("Error getting target file information for symbolic link\n");
            exit(-1);
        }

        removeExtension(filename);
        snprintf(buffer, sizeof(buffer), "Link name: %s\n"
                                         "Link size: %ld\n"
                                         "Target file size: %ld\n"
                                         "User rights: %s\n"
                                         "Group rights: %s\n"
                                         "Other rights: %s\n\n",
                 filename, fileInfo.st_size, targetFileInfo.st_size, userRights, groupRights, otherRights);

        char statFileName[MAX_BUFFER_SIZE] = "";
        snprintf(statFileName, sizeof(statFileName), "%s/%s_statistics.txt", outputDirectory, filename);
        int fileDescriptorOut = open(statFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fileDescriptorOut == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if (strcmp(buffer, "") != 0) {
            if (write(fileDescriptorOut, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(fileDescriptorOut);
                exit(EXIT_FAILURE);
            }
        }

        close(fileDescriptorOut);
    }
}
void processEntry(const char *entryName, const char *inputDirectory, const char *outputDirectory, const char scriptArgument) {
    char inputFilePath[MAX_PATH_LENGTH];
    snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", inputDirectory, entryName);

    int regFilePipe[2];
    if (pipe(regFilePipe) < 0) {
        perror("Error creating pipe\n");
        exit(EXIT_FAILURE);
    }

    pid_t childPid1 = fork();

    if (childPid1 == -1) {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    } else if (childPid1 == 0) {

        if (strstr(entryName, ".bmp") != NULL) {
            processImage(inputFilePath, outputDirectory, entryName);
            exit(EXIT_SUCCESS);
        } else {
            processFile(inputFilePath, outputDirectory, entryName);
            if (isRegularFile(inputFilePath) == 0) {
                exit(EXIT_SUCCESS);
            }

            close(regFilePipe[0]);
            dup2(regFilePipe[1], STDOUT_FILENO);
            close(regFilePipe[1]);
            execlp("cat", "cat", inputFilePath, (char *)NULL);

            perror("Error executing cat command\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (strstr(entryName, ".bmp") != NULL) {
            pid_t childPid2 = fork();

            if (childPid2 == -1) {
                perror("Error forking\n");
                exit(EXIT_FAILURE);
            } else if (childPid2 == 0) {
                grayscaleImage(inputFilePath);
                exit(EXIT_SUCCESS);
            } else {
                int status;
                pid_t who;
                who = wait(&status);

                if (WIFEXITED(status)) {
                    printf("Process with pid %d exited with code %d\n", who, WEXITSTATUS(status));
                } else {
                    printf("Process with pid %d and code %d did not exit normally\n", who, WEXITSTATUS(status));
                }

                who = wait(&status);
                if (WIFEXITED(status)) {
                    printf("Process with pid %d exited with code %d\n", who, WEXITSTATUS(status));
                } else {
                    printf("Process with pid %d and code %d did not exit normally\n", who, WEXITSTATUS(status));
                }
            }
        } else if (isRegularFile(inputFilePath) != 0) {
            int status;
            pid_t who;
            who = wait(&status);

            if (WIFEXITED(status)) {
                printf("Process with pid %d exited with code %d\n", who, WEXITSTATUS(status));
            } else {
                printf("Process with pid %d and code %d did not exit normally\n", who, WEXITSTATUS(status));
            }

            pid_t childPid3 = fork();

            if (childPid3 == -1) {
                perror("Error forking\n");
                exit(EXIT_FAILURE);
            } else if (childPid3 == 0) {
                close(regFilePipe[1]);
                dup2(regFilePipe[0], STDIN_FILENO);
                char scriptArgs[2];
                scriptArgs[0] = scriptArgument;
                scriptArgs[1] = '\0';
                execlp("bash", "bash", "propozitii.sh", scriptArgs, (char *)NULL);

                perror("Error executing propozitii.sh script\n");
                exit(EXIT_FAILURE);
            } else {
                close(regFilePipe[0]);
                int status;
                pid_t who;
                who = wait(&status);

                if (WIFEXITED(status)) {
                    printf("Process with pid %d exited with code %d\n", who, WEXITSTATUS(status));
                    printf("A total of %d correct sentences containing the character %c were identified\n", totalLines, scriptArgument);
                } else {
                    printf("Process with pid %d and code %d did not exit normally\n", who, WEXITSTATUS(status));
                }
            }
        } else {
            int status;
            pid_t who;
            who = wait(&status);

            if (WIFEXITED(status)) {
                printf("Process with pid %d exited with code %d\n", who, WEXITSTATUS(status));
            } else {
                printf("Process with pid %d and code %d did not exit normally\n", who, WEXITSTATUS(status));
            }
        }
    }
}

void processDirectory(const char *inputDirectory, const char *outputDirectory, const char scriptArgument) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(inputDirectory);
    if (dir == NULL) {
        perror("Error opening input directory\n");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            processEntry(entry->d_name, inputDirectory, outputDirectory, scriptArgument);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("Usage: ./program <input_directory> <output_directory> <c>\n");
        exit(EXIT_FAILURE);
    }

    const char *inputDirectory = argv[1];
    const char *outputDirectory = argv[2];
    const char scriptArgument = argv[3][0];

    processDirectory(inputDirectory, outputDirectory, scriptArgument);

    return 0;
}