#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000

void error(char *msg)
{
    printf("Error, %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error("wrong number of arguments");
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        error("could not open file");
    }

    FILE *sort_input = popen("sort", "w");
    if (sort_input == NULL)
    {
        error("could not open pipe for input");
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        fputs(line, sort_input);
    }

    fclose(fp);
    if (pclose(sort_input) != 0)
    {
        error("could not close pipe");
    }

    return 0;
}