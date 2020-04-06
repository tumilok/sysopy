#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        error("wrong number of arguments");
    }

    char *p_path = argv[1];
    char *f_path = argv[2];
    int N = atoi(argv[3]);

    FILE *fp = fopen(f_path, "w");
    if (fp == NULL)
    {
        error("could not open file");
    }

    FILE *pipe = fopen(p_path, "r");
    if (pipe == NULL)
    {
        fclose(fp);
        error("could not open file");
    }

    char buf[N];
    while(fgets(buf, N, pipe) != NULL)
    {
        fprintf(fp, buf, strlen(buf));
    }

    fclose(fp);
    fclose(pipe);

    return 0;
}