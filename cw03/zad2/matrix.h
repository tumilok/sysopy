#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Matrix
{
    FILE *fp;
    unsigned int row_num;
    unsigned int col_num;

} Matrix;

int get_row_num(FILE *fp);
int get_col_num(FILE *fp);
Matrix *init_matrix(char *fpath);
void free_matrix(Matrix *matrix);

#endif