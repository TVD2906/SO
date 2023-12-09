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
#define MAX_BUFFER_SIZE 4096

void formatPermissions(struct stat fileInfo, char* user, char* group, char* other) {
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

void removeFileExtension(const char *fileName) {
    char *lastDot = strrchr(fileName, '.');
    if (lastDot != NULL) {
        *lastDot = '\0';
    }
}

void processImageFile(const char *inputFilePath, const char *outputDirectory, const char* name, const char* parentFolder) {
    int file = open(inputFilePath, O_RDWR);
    if (file == -1) {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    struct stat info;

    if (stat(inputFilePath, &info) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    if(lseek(file, 18, SEEK_SET) == -1) {
        perror("Error setting cursor\n");
        close(file);
        exit(EXIT_FAILURE);
    }

    int height = 0, width = 0;
    read(file, &width, 4);
    read(file, &height, 4);
    char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
    char date[20];
    char buffer[MAX_BUFFER_SIZE] = "";
    strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
    formatPermissions(info, user_rights, group_rights, other_rights);

    fflush(NULL);
    
    if(lseek(file, 0, SEEK_SET) == -1) {
        perror("Error resetting cursor to the beginning of the file\n");
        close(file);
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();

    if(child == -1) {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    }
    else if(child == 0) {
        if(lseek(file, HEADER_SIZE, SEEK_SET) == -1) {
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

        exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(child, &status, 0);

        if (WIFEXITED(status)) {
            printf("Process with pid %d has terminated with code %d\n", child, WEXITSTATUS(status));
        } else {
            printf("Process with pid %d did not terminate normally\n", child);
        }
    }

    removeFileExtension(name);
    snprintf(buffer, sizeof(buffer), "file name: %s\n"
                                     "height: %d\n"
                                     "width: %d\n"
                                     "size: %ld\n"
                                     "user id: %d\n"
                                     "last modification time: %s\n"
                                     "link count: %ld\n"
                                     "user access rights: %s\n"
                                     "group access rights: %s\n"
                                     "other access rights: %s\n\n", 
                name, height, width, info.st_size, info.st_uid, date, info.st_nlink, user_rights, group_rights, other_rights);

    char stat_file_name[MAX_BUFFER_SIZE] = "";
    snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistics.txt", outputDirectory, name);
    int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_descriptor_out == -1) {
        perror("Error opening output file\n");
        exit(-1);
    }

    if(strcmp(buffer, "") != 0) {
        if (write(file_descriptor_out, buffer, strlen(buffer)) == -1) {
            perror("Error writing to output file\n");
            close(file);
            close(file_descriptor_out);
            exit(EXIT_FAILURE);
        }
    }

    close(file);
    close(file_descriptor_out);
}

void processRegularFile(const char *inputFilePath, const char *outputDirectory, const char* name) {

    struct stat info;

    if (lstat(inputFilePath, &info) == -1) {
        perror("Error getting file information\n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE] = "";

    if(S_ISREG(info.st_mode))
    {
        strcpy(buffer, "");
        char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
        char date[20];
        strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
        formatPermissions(info, user_rights, group_rights, other_rights);

        removeFileExtension(name);
        snprintf(buffer, sizeof(buffer), "file name: %s\n"
                             "size: %ld\n"
                             "user id: %d\n"
                             "last modification time: %s\n"
                             "link count: %ld\n"
                             "user access rights: %s\n"
                             "group access rights: %s\n"
                             "other access rights: %s\n\n", 
        name, info.st_size, info.st_uid, date, info.st_nlink, user_rights, group_rights, other_rights);

        char stat_file_name[MAX_BUFFER_SIZE] = "";
        snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistics.txt", outputDirectory, name);
        int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_descriptor_out == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if(strcmp(buffer, "") != 0) {
            if (write(file_descriptor_out, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(file_descriptor_out);
                exit(EXIT_FAILURE);
            }
        }

        close(file_descriptor_out);
    }
    else if(S_ISDIR(info.st_mode)) {
        strcpy(buffer, "");
        char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
        formatPermissions(info, user_rights, group_rights, other_rights);

        removeFileExtension(name);
        snprintf(buffer, sizeof(buffer), "directory name: %s\n"
                                        "user id: %d\n"
                                        "user access rights: %s\n"
                                        "group access rights: %s\n"
                                        "other access rights: %s\n\n",
                name, info.st_uid, user_rights, group_rights, other_rights);
        
        char stat_file_name[MAX_BUFFER_SIZE] = "";
        snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistics.txt", outputDirectory, name);
        int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_descriptor_out == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if(strcmp(buffer, "") != 0) {
            if (write(file_descriptor_out, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(file_descriptor_out);
                exit(EXIT_FAILURE);
            }
        }

        close(file_descriptor_out);
    }
    else if(S_ISLNK(info.st_mode)) {
        strcpy(buffer, "");
        char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
        formatPermissions(info, user_rights, group_rights, other_rights);
        struct stat target_file_info;
        if(stat(inputFilePath, &target_file_info) == -1)
        {
            perror("Error getting target file information for symbolic link\n");
            exit(-1);
        }

        removeFileExtension(name);
        snprintf(buffer, sizeof(buffer), "link name: %s\n"
                                        "link size: %ld\n"
                                        "target file size: %ld\n"
                                        "user access rights: %s\n"
                                        "group access rights: %s\n"
                                        "other access rights: %s\n\n",
                name, info.st_size, target_file_info.st_size, user_rights, group_rights, other_rights);

        char stat_file_name[MAX_BUFFER_SIZE] = "";
        snprintf(stat_file_name, sizeof(stat_file_name), "%s/%s_statistics.txt", outputDirectory, name);
        int file_descriptor_out = open(stat_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_descriptor_out == -1) {
            perror("Error opening output file\n");
            exit(-1);
        }

        if(strcmp(buffer, "") != 0) {
            if (write(file_descriptor_out, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file\n");
                close(file_descriptor_out);
                exit(EXIT_FAILURE);
            }
        }

        close(file_descriptor_out);
    }
}

void processFileEntry(const char *entryName, const char *inputDirectory, const char *outputDirectory) {
    char inputFilePath[MAX_PATH_LENGTH];
    snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", inputDirectory, entryName);

    pid_t childPid = fork();

    if (childPid == -1) {
        perror("Error forking\n");
        exit(EXIT_FAILURE);
    } else if (childPid == 0) {
        if (strstr(entryName, ".bmp") != NULL) {
            processImageFile(inputFilePath, outputDirectory, entryName, inputDirectory);
            exit(EXIT_SUCCESS);
        } else {
            processRegularFile(inputFilePath, outputDirectory, entryName);
            exit(EXIT_SUCCESS);
        }
    } else {
        int status;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Process with pid %d has terminated with code %d\n", childPid, WEXITSTATUS(status));
        } else {
            printf("Process with pid %d did not terminate normally\n", childPid);
        }
    }
}

void processInputDirectory(const char *inputDirectory, const char *outputDirectory) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(inputDirectory);
    if (dir == NULL) {
        perror("Error opening input directory\n");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            processFileEntry(entry->d_name, inputDirectory, outputDirectory);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_directory> <output_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *inputDirectory = argv[1];
    const char *outputDirectory = argv[2];

    processInputDirectory(inputDirectory, outputDirectory);

    return 0;
}
