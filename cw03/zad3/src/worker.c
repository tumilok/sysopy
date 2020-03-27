#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <sys/file.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#include "matrix.h"

void print_usage(int id, int pid)
{
    struct rusage usage;
    getrusage(id, &usage);

    printf("Process: %d\n", pid);
    printf("\tUser CPU time:\t\t%lu.%06lu s\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
    printf("\tSystem CPU time:\t%lu.%06lu s\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
}  

void run_worker(Matrix *a_matrix, Matrix *b_matrix, Matrix *res_matrix, int start_col, int end_col, int time_limit)
{
    int n = a_matrix -> col_num;
    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    int finished_multiplications = 0;
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    struct timespec end = start;

    int row_counter = 0;
    int col_counter = start_col;

    read_row(a_matrix, row, 0);
    read_col(b_matrix, col, start_col);

    while (col_counter < end_col)
    {
        clock_gettime(CLOCK_REALTIME, &end);
        if ((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1e9) > time_limit)
        {
            break;
        }

        int result = multiply_vectors(row, col, n);

        flock(fileno(res_matrix -> fp), LOCK_EX);
        write_result(res_matrix, row_counter, col_counter, result);
        flock(fileno(res_matrix -> fp), LOCK_UN);
        
        finished_multiplications++;
        
        row_counter++;
        if (row_counter == a_matrix -> row_num)
        { 
            row_counter = 0;
            col_counter++;

            read_row(a_matrix, row, 0);

            if (col_counter < end_col)
            {
                read_col(b_matrix, col, col_counter);
            }
        }
        else
        {
            read_next_row(a_matrix, row);
        }
    }

    fclose(a_matrix -> fp);
    fclose(b_matrix -> fp);
    fclose(res_matrix -> fp);

    print_usage(RUSAGE_SELF, getpid());

    exit(finished_multiplications);
}

int main(int argc, char *argv[])
{
    char *a_fname = argv[1];
    char *b_fname = argv[2];
    char *res_fname = argv[3];

    int start_col = atoi(argv[4]);
    int end_col = atoi(argv[5]);

    int time_limit = atoi(argv[6]);
    int cpu_time_limit = atoi(argv[7]);
    int memo_limit = atoi(argv[8]);

    long int memo_limit_b = memo_limit * 1000000;

    // set cpu limits
    struct rlimit cpu_l;
    getrlimit (RLIMIT_CPU, &cpu_l);

    cpu_l.rlim_max = cpu_time_limit;
    setrlimit(RLIMIT_CPU, &cpu_l);

    // Set memory limits
    struct rlimit memo_l;
    getrlimit(RLIMIT_AS, &memo_l);
    
    memo_l.rlim_max = memo_limit_b;
    setrlimit(RLIMIT_AS, &memo_l);

    Matrix *a_matrix = init_matrix(a_fname);
    Matrix *b_matrix = init_matrix(b_fname);
    
    Matrix *res_matrix = malloc(sizeof(Matrix));

    res_matrix -> fp = fopen(res_fname, "r+");
    if (res_matrix -> fp == NULL)
    {
        printf("Cannot open file: %s\n", res_fname);
        exit(EXIT_FAILURE);
    }

    res_matrix -> row_num = a_matrix -> row_num;
    res_matrix -> col_num = b_matrix -> col_num;

    run_worker(a_matrix, b_matrix, res_matrix, start_col, end_col, time_limit);

    return 0;
}