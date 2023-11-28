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

int main(int argc, char *argv[]) 
{
    if (argc != 2 || strstr(argv[1], ".bmp") == NULL) 
    {
        perror("Usage ./program <fisier_intrare>\n");
        exit(-1);
    }

    char *fin = argv[1];

    int fdin = open(fin, O_RDONLY);
    if (fdin == -1) 
    {
        perror("Error opening input file\n");
        exit(-1);
    }

    int fdout = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fdout == -1) 
    {
        perror("Error opening output file\n");
        close(fdin);
        exit(-1);
    }

    struct stat fileInfo;
    if (fstat(fdin, &fileInfo) == -1) 
    {
        perror("Error getting file information\n");
        close(fdin);
        close(fdout);
        exit(-1);
    }

    if(lseek(fdin, 18, SEEK_SET) == -1)
    {
        perror("Error setting cursor\n");
        close(fdin);
        close(fdout);
        exit(-1);
    }

    int height = 0, width = 0;
    read(fdin, &width, 4);
    read(fdin, &height, 4);
    char user_rights[4] = "", group_rights[4] = "", other_rights[4] = ""; 
    char date[20];
    strftime(date, sizeof(date), "%d.%m.%Y", localtime(&fileInfo.st_mtime));
    rigths(fileInfo, user_rights, group_rights, other_rights);

    fflush(NULL);

    char output[1024];
    snprintf(output, sizeof(output), "nume fisier: %s\n"
                                     "inaltime: %d\n"
                                     "lungime: %d\n"
                                     "dimensiune: %ld\n"
                                     "identificatorul utilizatorului: %d\n"
                                     "timpul ultimei modificari: %s\n"
                                     "contorul de legaturi: %ld\n"
                                     "drepturi de acces user: %s\n"
                                     "drepturi de acces grup: %s\n"
                                     "drepturi de acces altii: %s\n", 
            fin, height, width, fileInfo.st_size, fileInfo.st_uid, date, fileInfo.st_nlink, user_rights, group_rights, other_rights);

    if (write(fdout, output, strlen(output)) == -1)
    {
        perror("Error writing to output file");
        close(fdin);
        close(fdout);
        exit(-1);
    }

    close(fdin);
    close(fdout);

    return 0;
}
