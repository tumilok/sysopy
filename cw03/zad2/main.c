#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "matrix.h"

int main(int argc, char *argv[])
{
        // 4 parameters: 1 - file with matreces, 2 - number of processes, 3 - number of seconds, 4 - result file type
    if (argc != 5)
    {
        printf("Error, wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    char *list_fpath = argv[1];
    int workers_num = atoi(argv[2]);
    int time_limit = atoi(argv[3]);
    char *exec_flag = argv[4];

    FILE *list_fp = fopen(list_fpath, "r");
    if (list_fp == NULL)
    {
        printf("Error, cannot open file: %s\n", list_fpath);
        exit(EXIT_FAILURE);
    }
    
    char a_fpath[PATH_MAX];
    char b_fpath[PATH_MAX];
    char c_fpath[PATH_MAX];

    if(fscanf(list_fp, "%s %s %s", a_fpath, b_fpath, c_fpath) != 3)
    {
        printf("Error, configuration file is invalid");
        exit(EXIT_FAILURE);
    }
    fclose(list_fp);

    Matrix *a_matrix = init_matrix(a_fpath);
    Matrix *b_matrix = init_matrix(b_fpath);

    if (a_matrix -> col_num != b_matrix -> row_num)
    {
        printf("%s column number is not equal to %s row number", a_fpath, b_fpath);
        exit(EXIT_FAILURE);
    }

    if (workers_num <= 0 || b_matrix < workers_num)
    {
        printf("Error, wrong number of worker processes");
        exit(EXIT_FAILURE);
    }

    if (strcmp(exec_flag, "-shared") == 0)
    {
        
    }
    else if (strcmp(exec_flag, "-separated") == 0)
    {

    }
    else
    {
        printf("Error, wrong execution flag. Try '-shared' or '-separated");
        exit(EXIT_FAILURE);
    }
}