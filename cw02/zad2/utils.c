#include "utils.h"

const char *format = "%Y-%m-%d %H:%M:%S";
int atime = -1, mtime = -1;
int maxdepth = 10000;
char asign, msign;


int check_time(int count, char sign, time_t file_time)
{
    time_t now;
    struct tm *time_info;
    time(&now);
    time_info = localtime(&now);
    time_t current_date = mktime(time_info);

    int diff = difftime(current_date, file_time) / 86400; // convert into days

    switch (sign)
    {
    case '+':
        if (diff > count)
        {
            return 1;
        }
        break;
    case '-':
        if (diff < count)
        {
            return 1;
        }
        break;
    case '=':
        if (diff == count)
        {
            return 1;
        }    
    }

    return 0;
}

void print_results(const char* fpath, const struct stat *sb)
{
    char ftype[64];

    if (S_ISREG(sb -> st_mode))
    {
        strcpy(ftype, "file");
    }
    else if (S_ISDIR(sb -> st_mode))
    {
        strcpy(ftype, "dir");
    }
    else if (S_ISLNK(sb -> st_mode))
    {
        strcpy(ftype, "slink");
    }
    else if (S_ISCHR(sb -> st_mode))
    {
        strcpy(ftype, "char dev");
    }
    else if (S_ISBLK(sb -> st_mode))
    {
        strcpy(ftype, "block dev");
    }
    else if (S_ISFIFO(sb -> st_mode))
    {
        strcpy(ftype, "fifo");
    }
    else if (S_ISSOCK(sb -> st_mode))
    {
        strcpy(ftype, "socket");
    }

    struct tm tm_modif_time;
    localtime_r(&sb -> st_mtime, &tm_modif_time);
    char modif_time_str[255];
    strftime(modif_time_str, 255, format, &tm_modif_time);

    struct tm tm_access_time;
    localtime_r(&sb -> st_atime, &tm_access_time);
    char access_time_str[255];
    strftime(access_time_str, 255, format, &tm_access_time);

    printf("%s | %s | size: %ld | mtime: %s | atime: %s | nlinks: %ld\n",
    ftype, fpath, sb -> st_size, modif_time_str, access_time_str, sb -> st_nlink);
}
