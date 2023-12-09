#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define MAX_BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024

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

void processDirectory(char* path, char* outputFilename) {
    DIR* directory = opendir(path);
    if (!directory) {
        perror("Unable to open directory %s\n");
        exit(-1);
    }

    int outputFileDescriptor = open("statistics.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outputFileDescriptor == -1) {
        perror("Error opening output file\n");
        exit(-1);
    }

    struct dirent* entry = NULL;
    struct stat info;
    int inputFileDescriptor = 0;
    char buffer[MAX_BUFFER_SIZE] = "";
    char pathToEntry[MAX_PATH_LENGTH] = "";

    while ((entry = readdir(directory))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        strcpy(buffer, "");
        snprintf(pathToEntry, sizeof(pathToEntry), "%s/%s", path, entry->d_name);
        if (lstat(pathToEntry, &info) == -1) {
            printf("Entry named: %s\n", entry->d_name);
            perror("Error getting file information\n");
            exit(-1);
        }

        if (S_ISREG(info.st_mode)) {
            if (strstr(entry->d_name, ".bmp") != NULL) {
                inputFileDescriptor = open(pathToEntry, O_RDONLY);
                if (inputFileDescriptor == -1) {
                    perror("Error opening input file\n");
                    exit(-1);
                }

                if (lseek(inputFileDescriptor, 18, SEEK_SET) == -1) {
                    perror("Error setting cursor\n");
                    close(inputFileDescriptor);
                    exit(-1);
                }

                int height = 0, width = 0;
                read(inputFileDescriptor, &width, 4);
                read(inputFileDescriptor, &height, 4);
                char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                formatPermissions(info, userRights, groupRights, otherRights);

                fflush(NULL);

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
                         entry->d_name, height, width, info.st_size, info.st_uid, date, info.st_nlink, userRights, groupRights, otherRights);

                close(inputFileDescriptor);
            } else {
                char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                formatPermissions(info, userRights, groupRights, otherRights);
                snprintf(buffer, sizeof(buffer), "file name: %s\n"
                                                 "size: %ld\n"
                                                 "user id: %d\n"
                                                 "last modification time: %s\n"
                                                 "link count: %ld\n"
                                                 "user access rights: %s\n"
                                                 "group access rights: %s\n"
                                                 "other access rights: %s\n\n",
                         entry->d_name, info.st_size, info.st_uid, date, info.st_nlink, userRights, groupRights, otherRights);
            }
        } else if (S_ISLNK(info.st_mode)) {
            char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
            formatPermissions(info, userRights, groupRights, otherRights);
            struct stat targetFileInfo;
            if (stat(pathToEntry, &targetFileInfo) == -1) {
                perror("Error getting target file information for symbolic link\n");
                exit(-1);
            }
            snprintf(buffer, sizeof(buffer), "link name: %s\n"
                                             "link size: %ld\n"
                                             "target file size: %ld\n"
                                             "user access rights: %s\n"
                                             "group access rights: %s\n"
                                             "other access rights: %s\n\n",
                     entry->d_name, info.st_size, targetFileInfo.st_size, userRights, groupRights, otherRights);
        } else if (S_ISDIR(info.st_mode)) {
            char userRights[4] = "", groupRights[4] = "", otherRights[4] = "";
            formatPermissions(info, userRights, groupRights, otherRights);
            snprintf(buffer, sizeof(buffer), "directory name: %s\n"
                                             "user id: %d\n"
                                             "user access rights: %s\n"
                                             "group access rights: %s\n"
                                             "other access rights: %s\n\n",
                     entry->d_name, info.st_uid, userRights, groupRights, otherRights);
        }

        if (strcmp(buffer, "") != 0) {
            if (write(outputFileDescriptor, buffer, strlen(buffer)) == -1) {
                perror("Error writing to output file");
                close(outputFileDescriptor);
                exit(-1);
            }
        }
    }

    close(outputFileDescriptor);
    closedir(directory);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Usage ./program <input_directory>\n");
        exit(-1);
    }

    processDirectory(argv[1], "statistics.txt");

    return 0;
}