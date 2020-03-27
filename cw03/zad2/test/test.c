#include <stdio.h> 
#include <stdlib.h> 

#include <time.h>
#include <unistd.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../src/matrix.h"

#define MIN_VALUE 1
#define MAX_VALUE 9
#define TES_DIR "test/"
#define TIME_LIMIT "10"
#define EXEC_FLAG "-shared"

int randInt(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

int generate_test(int test_number, int min_matrix_size, int max_matrix_size)
{
    printf("\n\t\tTest %d\n", test_number);

    char fname1 [PATH_MAX];
    char fname2 [PATH_MAX];
    char fname3 [PATH_MAX];
    char config_fname [PATH_MAX];

    sprintf(fname1, "%s-%d.txt", "a_matrix", test_number);
    sprintf(fname2, "%s-%d.txt", "b_matrix", test_number);
    sprintf(fname3, "%s-%d.txt", "c_matrix", test_number);
    sprintf(config_fname, "%s-%d.txt", "lista", test_number);

    FILE* config_fp = fopen(config_fname, "w");
    fprintf(config_fp, "%s %s %s", fname1, fname2, fname3);
    fclose(config_fp);

    printf("\n\tCreated configuration file\n");

    int m = randInt(min_matrix_size, max_matrix_size);
    int n = randInt(min_matrix_size, max_matrix_size);
    int k = randInt(min_matrix_size, max_matrix_size);

    Matrix* a_matrix = generate_matrix(fname1, m, n, MIN_VALUE, MAX_VALUE);
    Matrix* b_matrix = generate_matrix(fname2, n, k, MIN_VALUE, MAX_VALUE);

    printf("\tCreated test matrices\n");

    int workers_num = randInt(1, k);
    char s_workers_num[10];
    sprintf(s_workers_num, "%d", workers_num);

    printf("\n\tParameters:\n");
    printf("\t\tworkers:\t%d\n", workers_num);
    printf("\t\ttime limit:\t%ss\n", TIME_LIMIT);
    printf("\t\ta_matrix:\t%dx%d\n", m, n);
    printf("\t\tb_matrix:\t%dx%d\n", n, k);
    
    int pid = vfork();
    if (pid == 0)
    {
        printf("\n\tRunning main...\n");
                         
        execl("./main", "./main", config_fname, s_workers_num, TIME_LIMIT, EXEC_FLAG, NULL);
        
        exit(1);
    }
    else
    {
        int return_status;
        printf("\n\tTesting...\n");
        
        waitpid(pid, &return_status, 0);

        if (return_status != 0)
        {
            printf("\n\tRunning failed\n");
            return 0;
        }
    }

    a_matrix -> fp = fopen(fname1, "r");
    b_matrix -> fp = fopen(fname2, "r");
    FILE* result_fp  = fopen(fname3, "r");

    if (result_fp == NULL)
    {
        printf("\n\tTest failed.\n");
        return 0;
    }

    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    for (int i = 0; i < m; i++)
    {
        read_row(a_matrix, row, i);
        for (int j = 0; j < k; j++)
        {
            read_col(b_matrix, col, j);

            int expected = multiply_vectors(row, col, n);
            int actual;
            fscanf(result_fp, "%d", &actual);

            if (expected != actual)
            {
                printf("\n\tTest failed\n");
                printf("\tExpected %d and got %d on row: %d, column: %d\n", expected, actual, i, j);
                return 0;
            }
        }
    }

    free(row);
    free(col);

    free_matrix(a_matrix);
    free_matrix(b_matrix);
    fclose(result_fp);

    printf("\tTest %d passed.\n", test_number);
    printf("--------------------------------------------\n");
    
    return 1;
}

int main(int argc, char* argv[])
{
    int tests_number = atoi(argv[1]);
    int min_matrix_size = atoi(argv[2]);
    int max_matrix_size = atoi(argv[3]);

    srand(time(NULL));
    int passed = 0;

    printf("\n##############################################\n");
    printf("                    Testing\n");
    printf("##############################################\n");

    for (int i = 0; i < tests_number; i++)
    {
        passed += generate_test(i + 1, min_matrix_size, max_matrix_size);
    }

    printf("\n\tFinished testing - %d/%d passed.\n", passed, tests_number);
    printf("##############################################\n");

    return 0;
}