#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

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

    srand(time(NULL));

    FILE *fp = fopen(f_path, "r");
    if (fp == NULL)
    {
        error("could not open file");
    }

    FILE *pipe = fopen(p_path, "w");
    if (pipe == NULL)
    {
        fclose(fp);
        error("could not open file");
    }

    char buf[N];
    while (fread(buf, sizeof(char), N, fp) > 0)
    {
        sleep(rand() % 3 + 1);
        char msg[N+15];
        sprintf(msg, "#%d#%s\n", getpid(), buf);
        fwrite(msg, 1, strlen(msg), pipe);
    }

    fclose(fp);
    fclose(pipe);

    return 0;
}