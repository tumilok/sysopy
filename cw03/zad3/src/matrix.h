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

Matrix *generate_matrix(char *fname, int row_num, int col_num, int min, int max);

int multiply_vectors(int* row, int* col, int n);

void move_pointer_to_line(FILE *fp, int row_number);

void read_row(Matrix *matrix, int *result_row, int row_number);

void read_next_row(Matrix* matrix, int* nums);

void read_col(Matrix *matrix, int *result_col, int col_number);

int get_row_num(FILE *fp);

int get_col_num(FILE *fp);

Matrix *init_matrix(char *fpath);

Matrix *create_result_matrix(char *fname, int row_num, int col_num);

void write_result(Matrix* matrix, int row, int col, int res);

int finsert(FILE* file, const char *buffer);

void free_matrix(Matrix *matrix);

#endif