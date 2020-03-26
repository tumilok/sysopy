#include <stdio.h> 
#include <stdlib.h> 

#include <time.h>
#include <unistd.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "matrix.h"

#define MIN 1
#define MAX 4
#define TES_DIR "test/"
#define MAX_EXEC_TIME "5"
#define EXECUTION_FLAG "-shared"

int generate_test(int min, int max, int i)
{
    char fname1 [PATH_MAX];
    char fname2 [PATH_MAX];
    char fname3 [PATH_MAX];
    char config_fname [PATH_MAX];

    printf("Test %d\n", i);

    sprintf(fname1, "%s-%d.txt", "matrixA", i);
    sprintf(fname2, "%s-%d.txt", "matrixB", i);
    sprintf(fname3, "%s-%d.txt", "matrixX", i);
    sprintf(config_fname, "%s-%d.txt", "lista", i);

    FILE* config_fp = fopen(config_fname, "w");
    fprintf(config_fp, "%s %s %s", fname1, fname2, fname3);
    fclose(config_fp);

    int m = (rand() % (min - max + 1)) + min;
    int n = (rand() % (min - max + 1)) + min;
    int k = (rand() % (min - max + 1)) + min;

    Matrix* matrixA = generate_matrix(fname1, m, n, MIN, MAX);
    Matrix* matrixB = generate_matrix(fname2, n, k, MIN, MAX);

    printf("\tGenerated test matrices\n");
    
    int pid = vfork();
    if (pid == 0)
    {
        int workers_num = (rand() % (1 - k + 1)) + 1;
        char s_workers_num[10];
        sprintf(s_workers_num, "%d", workers_num);

        printf("\tRunning main... with %d workers and %ss maximum execution time, matrixA: %dx%d, matrixB: %dx%d\n",
                         workers_num, MAX_EXEC_TIME, m, n, n, k);
                         
        execl("./main", "./main", config_fname, s_workers_num, MAX_EXEC_TIME, EXECUTION_FLAG, NULL);
        
        exit(1);
    }
    else
    {
        int return_status;
        printf("\tTesting...\n");
        waitpid(pid, &return_status, 0);

        /*if (return_status != 0)
        {
            printf("\tRunning failed\n");
            return 0;
        }*/
    }

    // Check if results are succesfull
    matrixA -> fp = fopen(fname1, "r");
    matrixB -> fp = fopen(fname2, "r");
    FILE* result_fp  = fopen(fname3, "r");

    if (result_fp = NULL)
    {
        printf("\tTest failed.\n");
        return 0;
    }

    int* row = malloc(sizeof(int) * n);
    int* col = malloc(sizeof(int) * n);

    for (int i = 0; i < m; i++)
    {
        read_row(matrixA, row, i);
        for (int j = 0; j < k; j++)
        {
            read_col(matrixB, col, j);

            int expected = multiply_vectors(row, col, n);
            int actual;
            fscanf(result_fp, "%d", &actual);

            if (expected != actual)
            {
                printf("\tTest failed\n");
                printf("\tExpected %d and got %d on row: %d, column: %d\n", expected, actual, i, j);
                return 0;
            }
        }
    }

    free(row);
    free(col);
    fclose(matrixA -> fp);
    fclose(matrixB -> fp);
    fclose(result_fp);

    printf("\tTest passed.\n");
    
    return 1;
}

int main(int argc, char* argv[])
{
    int testNumber = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);

    srand(time(NULL));
    int passed = 0;

    for (int i = 1; i <= testNumber; i++) {
        passed += generate_test(min, max, i);
    }

    printf("\nFinished testing - %d/%d passed.\n", passed, testNumber);
    printf("---------------------------------------\n");

    return 0;
}