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

void rigths(struct stat fileInfo, char* user, char* group, char* other)
{
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

void parcurgere_director(char* path, char* output_filename)
{
    DIR* directory = opendir(path);
    if(!directory)
    {
        perror("Unable to open directory %s\n");
        exit(-1);
    }

    int file_descriptor_out = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_descriptor_out == -1) 
    {
        perror("Error opening output file\n");
        exit(-1);
    }

    struct dirent* entry = NULL;
    struct stat info;
    int file_descriptor = 0;
    char buffer[MAX_BUFFER_SIZE] = "";
    char path_to_entry[MAX_PATH_LENGTH] = "";

    while((entry = readdir(directory)))
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        strcpy(buffer, "");
        snprintf(path_to_entry, sizeof(path_to_entry), "%s/%s", path, entry->d_name);
        if (lstat(path_to_entry, &info) == -1) 
        {
            printf("Entry named: %s\n", entry->d_name);
            perror("Error getting file information\n");
            exit(-1);
        }

        if(S_ISREG(info.st_mode))
        {
            if(strstr(entry->d_name, ".bmp") != NULL)
            {
                file_descriptor = open(path_to_entry, O_RDONLY);
                if (file_descriptor == -1) 
                {
                    perror("Error opening input file\n");
                    exit(-1);
                }

                if(lseek(file_descriptor, 18, SEEK_SET) == -1)
                {
                    perror("Error setting cursor\n");
                    close(file_descriptor);
                    exit(-1);
                }

                int height = 0, width = 0;
                read(file_descriptor, &width, 4);
                read(file_descriptor, &height, 4);
                char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                rigths(info, user_rights, group_rights, other_rights);

                fflush(NULL);

                snprintf(buffer, sizeof(buffer), "nume fisier: %s\n"
                                     "inaltime: %d\n"
                                     "lungime: %d\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n\n", 
                entry->d_name, height, width, info.st_size, info.st_uid, date, info.st_nlink, user_rights, group_rights, other_rights);

                close(file_descriptor);
            }
            else
            {
                char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
                char date[20];
                strftime(date, sizeof(date), "%d.%m.%Y", localtime(&info.st_mtime));
                rigths(info, user_rights, group_rights, other_rights);
                snprintf(buffer, sizeof(buffer), "nume fisier: %s\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n\n", 
                entry->d_name, info.st_size, info.st_uid, date, info.st_nlink, user_rights, group_rights, other_rights);
            }
        }
        else if(S_ISLNK(info.st_mode))
        {
            char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
            rigths(info, user_rights, group_rights, other_rights);
            struct stat target_file_info;
            if(stat(path_to_entry, &target_file_info) == -1)
            {
                perror("Error getting target file information for symbolic link\n");
                exit(-1);
            }
            snprintf(buffer, sizeof(buffer), "nume legatura: %s\n"
                                            "dimensiune legatura: %ld\n"
                                            "dimensiune fisier: %ld\n"
                                            "drepturi de acces user: %s\n"
                                            "drepturi de acces grup: %s\n"
                                            "drepturi de acces altii: %s\n\n",
                    entry->d_name, info.st_size, target_file_info.st_size, user_rights, group_rights, other_rights);
        }
        else if(S_ISDIR(info.st_mode))
        {
            char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
            rigths(info, user_rights, group_rights, other_rights);
            snprintf(buffer, sizeof(buffer), "nume director: %s\n"
                                            "identificatorul utilizatorului: %d\n"
                                            "drepturi de acces user: %s\n"
                                            "drepturi de acces grup: %s\n"
                                            "drepturi de acces altii: %s\n\n",
                    entry->d_name, info.st_uid, user_rights, group_rights, other_rights);
        }

        if(strcmp(buffer, "") != 0)
        {
            if (write(file_descriptor_out, buffer, strlen(buffer)) == -1)
            {
                perror("Error writing to output file");
                close(file_descriptor_out);
                exit(-1);
            }
        }
    }
    
    close(file_descriptor_out);
    closedir(directory);
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        perror("Usage ./program <director_intrare>\n");
        exit(-1);
    }

    parcurgere_director(argv[1], "statistica.txt");

    return 0;
}
