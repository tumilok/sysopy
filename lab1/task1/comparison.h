#ifndef COMPARISON_H
#define COMPARISON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILENAME "tmp.txt"

struct Block
{
    char **block_array;
    int block_length;
};

struct ArrayOfBlocks
{
    struct Block *main_array;
    int array_length;
};

char *str_cat(const char *str1, const char *str2);
void compare_files(const char *fname1, const char *fname2);
int count_block_operations();
char **init_block_operations();
int add_operation_block(struct ArrayOfBlocks *array);
struct ArrayOfBlocks init_main_array();
void print_arrays(struct ArrayOfBlocks *array);
int get_block_length(struct Block *block);
int delete_operation(struct Block *block, int index);
int delete_operation_block(struct ArrayOfBlocks *array, int index);

#endif