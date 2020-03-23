#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

void read_parameters(int argc, char *argv[])
{
    // 4 parameters: 1 - file with matreces, 2 - number of processes, 3 - number of seconds, 4 - result file type
    if (argc != 5)
    {
        printf("Error, wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    char *list_fpath = argv[1];
    FILE *list_fp = fopen(list_fpath, "r");
    if (list_fp == NULL)
    {
        printf("Error, cannot open file: %s\n", list_fpath);
        exit(EXIT_FAILURE);
    }

    int child_processes = atoi(argv[2]);
    time_t time_limit = atoi(argv[3]);

    bool is_shared;
    if (strcmp(argv[4], "shared") == 0)
    {
        is_shared = true;
    }
    else if (strcmp(argv[4], "scattered") == 0)
    {
        is_shared = false;
    }
    else
    {
        printf("Error, wrong parameter: %s\n", argv[4]);
        exit(EXIT_FAILURE);
    }

    printf("Path to lista: %s\n", list_fpath);
    printf("Number of children: %d\n", child_processes);
    printf("Life time in save_mode: %ld\n", time_limit);
    printf("Saving result matrix definition: %s, %d\n", argv[4], is_shared);
    
    char *a_fpath = (char*) calloc(100, sizeof(char));
    char *b_fpath = (char*) calloc(100, sizeof(char));
    char *c_fpath = (char*) calloc(100, sizeof(char));

    strcpy(a_fpath, "./");
    strcpy(b_fpath, "./");
    strcpy(c_fpath, "./");

    char buf[10];
    int file_number = 1;
    while (fscanf(list_fp, "%s", buf) != EOF)
    {
        if (file_number == 1)
        {
            strcat(a_fpath, buf);
        }
        else if (file_number == 2)
        {
            strcat(b_fpath, buf);
        }
        else
        {
            strcat(c_fpath, buf);
        }
        file_number++;
    }
    fclose(list_fp);

    printf("path of matrix a: %s\n", a_fpath);
    printf("path of matrix b: %s\n", b_fpath);
    printf("path of matrix c: %s\n", c_fpath);

    FILE *a_fp = fopen(a_fpath, "r");
    if (a_fp == NULL)
    {
        printf("Error, cannot open file %s", a_fpath);
        exit(EXIT_FAILURE);
    }

    FILE *b_fp = fopen(b_fpath, "r");
    if (a_fp == NULL)
    {
        printf("Error, cannot open file %s", b_fpath);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    read_parameters(argc, argv);
    return 0;
}