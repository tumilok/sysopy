#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Matrix
{
    FILE *fp;
    unsigned int raw_num;
    unsigned int col_num;

} Matrix;

int get_raw_num(FILE *fp)
{
    fseek(fp, 0, 0);
    char *line = NULL;
    size_t len = 0;
    int raw_num = 0;

    while (getline(&line, &len, fp) != -1)
    {
        raw_num++;
    }
    return raw_num;
}

int get_col_num(FILE *fp)
{
    fseek(fp, 0, 0);
    int col_num = 0;

    for (char c = getc(fp); c != '\n'; c = getc(fp))
    {
        if (c != ' ')
        {
            col_num++;
        }
    }
    return col_num;
}

void free_matrix(Matrix *matrix)
{
    fclose(matrix -> fp);
    free(matrix);
}

Matrix *init_matrix(char *fpath, unsigned int raw_num, unsigned int col_num)
{
    FILE *fp = fopen(fpath, "r");
    if (fp == NULL)
    {
        printf("Error, cannot open file: %s", fpath);
        exit(EXIT_FAILURE);
    }

    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> fp = fp;
    matrix -> raw_num = raw_num;
    matrix -> col_num = col_num;
}

int main()
{
    FILE *a_matrix = fopen("a.txt", "r");
    if (a_matrix == NULL)
    {
        printf("Error, cannot open file: a.txt");
        exit(EXIT_FAILURE);
    }

    int raw_num = get_raw_num(a_matrix);
    int col_num = get_col_num(a_matrix);

    fclose(a_matrix);

    Matrix *matrix = init_matrix("a.txt", raw_num, col_num);

    printf("number of raws: %d\n", matrix -> raw_num);
    printf("number of cols: %d\n", matrix -> col_num);

    free(matrix);

}