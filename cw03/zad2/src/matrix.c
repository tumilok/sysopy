#include "matrix.h"


Matrix *generate_matrix(char *fname, int row_num, int col_num, int min, int max)
{
    Matrix *matrix = malloc(sizeof(Matrix));

    matrix -> fp = fopen(fname, "w+");
    matrix -> row_num = row_num;
    matrix -> col_num = col_num;

    for (int i = 0; i < row_num; i++)
    {
        fprintf(matrix -> fp, "%d", (rand() % (min - max + 1)) + min);
        for (int j = 1; j < col_num; j++)
        {
            fprintf(matrix -> fp, " %d", (rand() % (min - max + 1)) + min);
        }
        fputc('\n', matrix -> fp);
    }
    fclose(matrix -> fp);
    
    return matrix;
}

int multiply_vectors(int* row, int* col, int n)
{
    int result = 0;
    for (int i = 0; i < n; i++)
    {
        result += row[i] * col[i];
    }
    return result;
}

void move_pointer_to_line(FILE *ptr, int row_number)
{
    fseek(ptr, 0, SEEK_SET);

    int count = 0;
    while (count < row_number)
    {
        for (char c = getc(ptr); c != '\n'; c = getc(ptr)) {}
        count++;
    }
}

void read_row(Matrix *matrix, int *result_row, int row_number)
{
    move_pointer_to_line(matrix -> fp, row_number);

    for (int i = 0; i < matrix -> col_num; i++)
    {
        fscanf(matrix -> fp, "%d", &(result_row[i]));
    }
}

void read_next_row(Matrix* matrix, int* nums)
{
    for (int i = 0; i < matrix -> col_num; i++)
    {
        fscanf(matrix -> fp, "%d", &(nums[i]));
    }
}

void read_col(Matrix *matrix, int *result_col, int col_number)
{
    move_pointer_to_line(matrix -> fp, 0);

    for (int i = 0; i < matrix -> row_num; i++)
    {
        int j = 0;
        int num;
        
        while (j <= col_number)
        {
            fscanf(matrix -> fp, "%d", &num);
            j++;
        }
        result_col[i] = num;

        for (char c = getc(matrix -> fp); c != '\n'; c = getc(matrix -> fp)) {}
    }
}

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
        printf("Error, cannot open file: %s\n", fpath);
        exit(EXIT_FAILURE);
    }
    matrix -> row_num = get_row_num(matrix -> fp);
    matrix -> col_num = get_col_num(matrix -> fp);

    return matrix;
}

Matrix *create_result_matrix(char *fname, int row_num, int col_num)
{
    Matrix *result_matrix = malloc(sizeof(Matrix));

    result_matrix -> fp = fopen(fname, "w+");
    if (result_matrix -> fp == NULL)
    {
        printf("Error, cannot open/create result file %s\n", fname);
        exit(EXIT_FAILURE);
    }

    result_matrix -> row_num = row_num;
    result_matrix -> col_num = col_num;

    for(int j = 0; j < row_num; j++) {
        for(int i = 1; i < col_num; i++) {
            fputc(' ', result_matrix -> fp);
        }
        fputc('\n', result_matrix -> fp);
    }

    return result_matrix;
}

int finsert(FILE* file, const char *buffer)
{
    long int insert_pos = ftell(file);
    if (insert_pos < 0)
    {
        return insert_pos;
    }

    int seek_ret = fseek(file, 0, SEEK_END);
    if (seek_ret)
    {
        return seek_ret;
    }
    
    long int total_left_to_move = ftell(file);
    if (total_left_to_move < 0)
    {
        return total_left_to_move;
    }

    char move_buffer[1024];
    long int ammount_to_grow = strlen(buffer);
    if (ammount_to_grow >= sizeof(move_buffer))
    {
        return -1;
    }

    total_left_to_move -= insert_pos;

    while(1 == 1)
    {
        int ammount_to_move = sizeof(move_buffer);
        if (total_left_to_move < ammount_to_move)
        {
            ammount_to_move = total_left_to_move;
        }

        long int read_pos = insert_pos + total_left_to_move - ammount_to_move;

        seek_ret = fseek(file, read_pos, SEEK_SET);
        if (seek_ret)
        {
            return seek_ret;
        }
        
        fread(move_buffer, ammount_to_move, 1, file);
        if (ferror(file))
        {
            return ferror(file);
        }

        seek_ret = fseek(file, read_pos + ammount_to_grow, SEEK_SET);
        if (seek_ret)
        {
            return seek_ret;
        }

        fwrite(move_buffer, ammount_to_move, 1, file);
        if (ferror(file))
        {
            return ferror(file);
        }

        total_left_to_move -= ammount_to_move;

        if (!total_left_to_move)
        {
            break;
        }
    }

    seek_ret = fseek(file, insert_pos, SEEK_SET);
    if (seek_ret)
    {
        return seek_ret;
    }
    
    fwrite(buffer, ammount_to_grow, 1, file);
    if (ferror(file))
    {
        return ferror(file);
    }

    return 0;
}

void write_result(Matrix* matrix, int row, int col, int res)
{
    move_pointer_to_line(matrix -> fp, row);

    int spaces = 0;
    while (spaces < col)
    {
        if (getc(matrix -> fp) == ' ')
        {
            spaces++;
        }
    }

    char str_num[10];
    sprintf(str_num, "%d", res);

    finsert(matrix -> fp, str_num);
    if (ferror(matrix -> fp) != 0)
    {
        printf("Error");
        exit(EXIT_FAILURE);
    }
    fflush(matrix -> fp);
}

void free_matrix(Matrix *matrix)
{
    fclose(matrix -> fp);
    free(matrix);
}