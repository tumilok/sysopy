#include "matrix.h"

int get_row_num(FILE *fp)
{
    fseek(fp, 0, 0);
    char *line = NULL;
    size_t len = 0;
    int row_num = 0;

    while (getline(&line, &len, fp) != -1)
    {
        row_num++;
    }
    return row_num;
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

Matrix *init_matrix(char *fpath)
{
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix -> fp = fopen(fpath, "r");
    if (matrix -> fp == NULL)
    {
        printf("Error, cannot open file: %s", fpath);
        exit(EXIT_FAILURE);
    }
    matrix -> row_num = get_row_num(matrix -> fp);
    matrix -> col_num = get_row_num(matrix -> fp);

    return matrix;
}

void free_matrix(Matrix *matrix)
{
    fclose(matrix -> fp);
    free(matrix);
}