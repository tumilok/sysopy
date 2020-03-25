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

void run_simple_worker(Matrix *a_matrix, Matrix *b_matrix, char *result_fname, int time_limit, int start_col, int end_col)
{
    FILE *fp = fopen(result_fname, "w");

    int n = a_matrix -> col_num;
    int *row = malloc(n * sizeof(int));
    int *col = malloc(n * sizeof(int));

    int finished_multiplications = 0;
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    struct timespec end = start;

    int row_counter = 0;
    int col_counter = start_col;

    read_row(a_matrix, row, 0);
    read_col(b_matrix, col, start_col);

    while (row_counter < a_matrix -> row_num)
    {
        clock_gettime(CLOCK_REALTIME, &end);
        if ((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1e9) > time_limit)
        {
            break;
        }

        int result = multiply_vectors(row, col, n);

        fprintf(fp, "%d", result);

        finished_multiplications++;
        
        col_counter++;
        if (col_counter == end_col)
        {
            col_counter = start_col;
            row_counter++;
            fputc('\n', fp);

            if (row_counter < a_matrix -> row_num)
            {
                read_next_row(a_matrix, row);
            }
        }
        else
        {
            fputc(' ', fp);
        }

        read_col(b_matrix, col, col_counter);
    }

    fclose(fp);
    
    exit(finished_multiplications);
}

void separated_manager(char *a_fpath, char *b_fpath, char *c_fpath, int workers_num, char *time_limit)
{
    Matrix *a_matrix = init_matrix(a_fpath);
    Matrix *b_matrix = init_matrix(b_fpath);

    check_matrices(a_matrix, b_matrix, workers_num);

    int *workers_pids = malloc(workers_num * sizeof(int));
    char **files = malloc((workers_num + 1) * sizeof(char*));
    double cur_position = 0.0;
    double section = (double) b_matrix -> col_num / workers_num;

    for (int i = 0; i < workers_num; i++)
    {
        int start = (int) cur_position;
        cur_position += section;
        int end = (int) cur_position;
        files[i] = malloc(150 * sizeof(char));
        sprintf(files[i], "worker-%d", start);

        int forked = fork();
        if (forked == 0)
        {
            if (i == workers_num - 1)
            {
                end = b_matrix -> col_num;
            }
            run_simple_worker(a_matrix, b_matrix, files[i], atoi(time_limit), start, end);
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

    files[workers_num] = NULL;

    char**args = malloc(sizeof(char*) * (workers_num + 3));

    args[0] = "paste";
    args[1] = "-d ";
    printf("%d\n",workers_num);
    for(int i = 2; i <= workers_num + 1; i++) {
        args[i] = files[i - 2];
    }
    args[workers_num + 2] = NULL;

    int v_pid = vfork();
    if (v_pid== 0) {
        int fd = open(c_fpath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fd, 1);
        close(fd);
        execv("/usr/bin/paste", args);
    }

    wait(NULL);

    for (int i = 0; i < workers_num; i++) {
        remove(files[i]);
    }

    exit(0);
}

void shared_manager(char *a_fpath, char *b_fpath, char *c_fpath, int workers_num, char *time_limit)
{
    Matrix *a_matrix = init_matrix(a_fpath);
    Matrix *b_matrix = init_matrix(b_fpath);

    check_matrices(a_matrix, b_matrix, workers_num);

    Matrix *c_matrix = create_result_matrix(c_fpath, a_matrix -> row_num, b_matrix -> col_num);

    fclose(a_matrix -> fp);
    fclose(b_matrix -> fp);
    fclose(c_matrix -> fp);

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

    free(a_matrix);
    free(b_matrix);
    free(c_matrix);
    free(workers_pids);
}

int main(int argc, char *argv[])
{
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
        separated_manager(a_fpath, b_fpath, c_fpath, workers_num, time_limit);
    }
    else
    {
        printf("Error, wrong execution flag. Try '-shared' or '-separated\n");
        exit(EXIT_FAILURE);
    }
}