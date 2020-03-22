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
    if (S_ISDIR(sb -> st_mode))
    {
        pid_t pid_fork = fork();

        if (pid_fork < 0)
        {
            printf("Cannot fork\n");
            exit(EXIT_FAILURE);
        }
        else if(pid_fork == 0)
        {
            
            // get process ID (PID) of the calling process
            printf("DIRECTORY: %s, PID: %d\n", fpath, getpid());
            
            // replace the current process image with a new process image
            int exec_status = execlp("ls", "ls", "-l", fpath, NULL);
            if (exec_status != 0)
            {
                printf("Exec failed");
                exit(EXIT_FAILURE);
            }
            exit(exec_status);
        }
        else
        {
            wait(0);
        }
    }
    return 0;
}

void dir_info_stat(char *path){

    if (path == NULL){
        return;
    }
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        printf("Cannot open directory\n");
        exit(EXIT_FAILURE);
    }

    struct stat buffer;
    lstat(path, &buffer);
    if (S_ISDIR(buffer.st_mode))
    {
        pid_t pid_fork = fork();

        if (pid_fork < 0)
        {
            printf("Cannot fork\n");
            exit(EXIT_FAILURE);
        }
        else if(pid_fork == 0)
        {
            
            // get process ID (PID) of the calling process
            printf("DIRECTORY: %s, PID: %d\n", path, getpid());
            
            // replace the current process image with a new process image
            int exec_status = execlp("ls", "ls", "-l", path, NULL);
            if (exec_status != 0)
            {
                printf("Exec failed");
                exit(EXIT_FAILURE);
            }
            exit(exec_status);
        }
        else
        {
            wait(0);
        }
    }
    
    
    struct dirent *file;
    char new_path[256];
    while ((file = readdir(dir)) != NULL)
    {
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);

        // check information about a file
        if (lstat(new_path, &buffer) < 0)
        {
            printf("Cannot lstat file %s: ", new_path);
            exit(EXIT_FAILURE);
        }
        
        // check if file is a directory
        if (S_ISDIR(buffer.st_mode))
        {

            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0)
            {
                continue;
            }
            find_dir(new_path);
        }
    }
    closedir(dir);
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