#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "matrix.h"

void check_matrices(Matrix *a_matrix, Matrix *b_matrix, int workers_num)
{
    if (a_matrix -> col_num != b_matrix -> row_num)
    {
        printf("Error, column number of the first matrix is not equal to row number of the second matrix\n");
        exit(EXIT_FAILURE);
    }

    if (workers_num <= 0 || b_matrix -> col_num < workers_num)
    {
        printf("Error, wrong number of worker processes\n");
        exit(EXIT_FAILURE);
    }
}

void shared_manager(char *a_fpath, char *b_fpath, char *c_fpath, int workers_num, char *time_limit)
{
    Matrix *a_matrix = init_matrix(a_fpath);
    Matrix *b_matrix = init_matrix(b_fpath);

    check_matrices(a_matrix, b_matrix, workers_num);

    Matrix *c_matrix = create_result_matrix(c_fpath, a_matrix -> row_num, b_matrix -> col_num);

    free_matrix(a_matrix);
    free_matrix(b_matrix);
    free_matrix(c_matrix);

    int *workers_pids = malloc(workers_num * sizeof(int));
    double cur_position = 0.0;
    double section = (double) b_matrix -> col_num / workers_num;

    for (int i = 0; i < workers_num; i++)
    {
        int start = (int) cur_position;
        cur_position += section;
        int end = (int) cur_position;

        int forked = fork();
        if (forked == 0)
        {
            printf("Child");
            if (i == workers_num - 1)
            {
                end = b_matrix -> col_num;
            }

            char s_start[10];
            char s_end[10];

            sprintf(s_start, "%d", start);
            sprintf(s_end, "%d", end);

            execl("./worker", "./worker", a_fpath, b_fpath, c_fpath, s_start, s_end, time_limit, NULL);
        }
        else
        {
            workers_pids[i] = forked;
        }
    }

    for (int i = workers_num - 1; i >= 0; i--) 
    {
        int return_status;
        waitpid(workers_pids[i], &return_status, 0);
        printf("Process %d ended with status: %d\n", workers_pids[i], WEXITSTATUS(return_status));
    }
}

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
    char *time_limit = argv[3];
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
        printf("Error, configuration file is invalid\n");
        exit(EXIT_FAILURE);
    }
    fclose(list_fp);

        if (strcmp(exec_flag, "-shared") == 0)
    {
        shared_manager(a_fpath, b_fpath, c_fpath, workers_num, time_limit);
    }
    else if (strcmp(exec_flag, "-separated") == 0)
    {

    }
    else
    {
        printf("Error, wrong execution flag. Try '-shared' or '-separated\n");
        exit(EXIT_FAILURE);
    }
}