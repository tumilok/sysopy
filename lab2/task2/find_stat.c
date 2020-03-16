#include "find_stat.h"


int check_conditions(struct stat* dir_stat)
{
    if (atime != -1 && check_time(atime, asign, dir_stat -> st_atime) == 0)
    {
        return 0;
    }
    
    if (mtime != -1 && check_time(mtime, msign, dir_stat -> st_mtime) == 0)
    {
        return 0;
    }
    return 1;
}

void find_dir(char *fpath, int depth)
{
    if (depth > maxdepth || fpath == NULL)
    {
        return;
    }

    DIR* dirp = opendir(fpath);
    if (dirp == NULL){
        printf("Cannot open directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *file;
    char new_path[256];

    while ((file = readdir(dirp)) != NULL)
    {
        strcpy(new_path, fpath);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);
        
        struct stat buffer;
        if (lstat(new_path, &buffer) < 0)
        {
            printf("Cannot lstat file %s: ", new_path);
            exit(EXIT_FAILURE);
        }
        
        if (S_ISDIR(buffer.st_mode))
        {
            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0)
            {
                continue;
            }

            find_dir(new_path, depth + 1);
        }

        if (check_conditions(&buffer) == 1)
        {
            print_results(new_path, &buffer);
        }
        
    }
    closedir(dirp);
}

void stat_exec()
{
    char* path = "../";
    maxdepth = 10;
    atime = 30;
    asign = '-';

    find_dir(path, 1);
}