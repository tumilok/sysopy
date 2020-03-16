#include "find_nftw.h"
#include "find_stat.h"

void read_parameters(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Wrong number of parameters");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "nftw") == 0)
    {
        nftw_exec();
    }
    else if (strcmp(argv[1], "stat") == 0)
    {
        stat_exec();
    }
    else
    {
        printf("Wrong parameters");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv)
{
    read_parameters(argc, argv);  

    return 0;
}