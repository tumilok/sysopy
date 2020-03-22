#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>


static int dir_info_nftw(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if (S_ISDIR(sb -> st_mode) != 0)
    {
        pid_t child_pid = fork();
        if (child_pid < 0)
        {
            printf("Error, while creating child process\n");
            exit(EXIT_FAILURE);
        }
        else if(child_pid == 0)
        {   
            printf("Dir path: %s, PID: %d\n", fpath, getpid());
            
            // replace the current process image with a new process image
            if (execlp("ls", "ls", "-l", fpath, NULL) != 0)
            {
                printf("Error, exec failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            wait(0);
        }
    }
    return 0;
}

void dir_info_stat(char *fpath){

    if (fpath == NULL)
    {
        return;
    }

    DIR* dirp = opendir(fpath);
    if (dirp == NULL)
    {
        printf("Cannot open directory\n");
        exit(EXIT_FAILURE);
    }

    struct stat buffer;
    lstat(fpath, &buffer);
    if (S_ISDIR(buffer.st_mode) != 0)
    {
        pid_t child_pid = fork();
        if (child_pid < 0)
        {
            printf("Error, while creating child process\n");
            exit(EXIT_FAILURE);
        }
        else if(child_pid == 0)
        {   
            printf("Dir path: %s, PID: %d\n", fpath, getpid());
            
            // replace the current process image with a new process image
            if (execlp("ls", "ls", "-l", fpath, NULL) != 0)
            {
                printf("Error, exec failed");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            wait(0);
        }
    }
    
    struct dirent *file;
    char new_fpath[256];

    while ((file = readdir(dirp)) != NULL)
    {
        strcpy(new_fpath, fpath);
        strcat(new_fpath, "/");
        strcat(new_fpath, file -> d_name);

        if (lstat(new_fpath, &buffer) < 0)
        {
            printf("Error, cannot lstat file %s: ", new_fpath);
            exit(EXIT_FAILURE);
        }
        
        if (S_ISDIR(buffer.st_mode) == 0)
        {
            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0)
            {
                continue;
            }
            dir_info_stat(new_fpath);
        }
    }
    closedir(dirp);
}

void read_parameters(int argc, char *argv[])
{                  
    if (argc < 3)
    {
        printf("Wrong number of parameters");
        exit(EXIT_FAILURE);
    }

    char *path = argv[1];

    if (strcmp(argv[2], "nftw") == 0)
    {
        nftw(path, dir_info_nftw, 10, FTW_PHYS);
    }
    else if (strcmp(argv[2], "stat") == 0)
    {
        dir_info_stat(path);
    }
    else 
    {
        printf("Wrong parametr: %s", argv[2]);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) 
{
    read_parameters(argc, argv);
    return 0;
}