#include "find_nftw.h"
#include "find_stat.h"

void read_parameters(int argc, char *argv[])
{                  
    if (argc < 3)
    {
        printf("Wrong number of parameters");
        exit(EXIT_FAILURE);
    }

    char *path = argv[1];

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "mtime") == 0)
        {
            i++;
            msign = argv[i][0];
            mtime = abs(atoi(argv[i]));
        }
        else if (strcmp(argv[i], "atime") == 0)
        {
            i++;
            asign = argv[i][0];
            atime = abs(atoi(argv[i]));
        }
        else if (strcmp(argv[i], "maxdepth") == 0)
        {
            i++;
            maxdepth = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "nftw") == 0)
        {
            nftw_exec(path);
        }
        else if (strcmp(argv[i], "stat") == 0)
        {
            stat_exec(path);
        }
        else
        {
            printf("Wrong argument: %s", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv)
{
    read_parameters(argc, argv);  

    return 0;
}