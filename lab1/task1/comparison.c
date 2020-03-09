#include "comparison.h"

char *str_cat(const char *str1, const char *str2)
{
    char *resstr = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(resstr, str1);
    strcat(resstr, str2);
    return resstr;
}

void compare_files(const char *fname1, const char *fname2)
{
    system(str_cat("diff ", str_cat(fname1, str_cat(" ", str_cat(fname2, str_cat(" > ", FILENAME))))));
}

int count_block_operations()
{
    FILE *fp = fopen(FILENAME, "r");
    if (!fp)
    {
        fprintf(stderr, "Error opening file '%s'\n", FILENAME);
        exit(EXIT_FAILURE);
    }
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    int operations_count = 0;
    ssize_t line_size = getline(&line_buf, &line_buf_size, fp);

    while (line_size >= 0)
    {
        if (isdigit(line_buf[0]))
        {
            operations_count++;
        }
        line_size = getline(&line_buf, &line_buf_size, fp);
    }

    free(line_buf);
    line_buf = NULL;

    fclose(fp);

    return operations_count;
}

char **init_block_operations()
{
    FILE *fp = fopen(FILENAME, "r");
    if (!fp)
    {
        fprintf(stderr, "Error opening file '%s'\n", FILENAME);
        exit(EXIT_FAILURE);
    }
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    ssize_t line_size = getline(&line_buf, &line_buf_size, fp);
    int operations_number = count_block_operations();
    char **block = calloc(operations_number, sizeof(char*));
    char *block_line;
    int i = 0;

    while (line_size >= 0)
    {
        if (isdigit(line_buf[0]))
        {
            if (i != 0)
            {
                block[i - 1] = block_line;
            }
            block_line = "";
            i++; 
        } 
        block_line = str_cat(block_line, line_buf);
        
        line_size = getline(&line_buf, &line_buf_size, fp);
    }
    block[i - 1] = block_line;

    free(line_buf);
    line_buf = NULL;

    fclose(fp);

    return block;
}

int add_operation_block(struct ArrayOfBlocks *array) {
    if (array == NULL)
    {
        fprintf(stderr, "Null pointer exception\n");
        return EXIT_FAILURE;
    }

    array -> array_length++;
    int length = array -> array_length;

    array -> main_array = (length == 1) ?
        calloc(length, sizeof(struct Block)) :
            realloc(array -> main_array, length * sizeof(struct Block));

    array -> main_array[length - 1].block_length = count_block_operations();
    array -> main_array[length - 1].block_array = init_block_operations();

    return length - 1;
}

struct ArrayOfBlocks init_main_array()
{
    struct ArrayOfBlocks array;
    array.array_length = 0;
    return array;
}

void print_arrays(struct ArrayOfBlocks *array)
{
    if (array == NULL)
    {
        fprintf(stderr, "Null pointer exception\n");
        return;
    }

    for (int i = 0; i < array -> array_length; i++)
    {
        for (int j = 0; j < array -> main_array[i].block_length; j++)
        {
            printf("%s", array -> main_array[i].block_array[j]);
        }
        printf("\n");
    }
}

int get_block_length(struct Block *block)
{
    if (block == NULL)
    {
        fprintf(stderr, "Null pointer exception\n");
        return EXIT_FAILURE;
    }

    int count_operations = 0;
    for (int i = 0; i < block -> block_length; i++)
    {
        if (block -> block_array[i] != NULL && isdigit(block -> block_array[i][0]))
        {
            count_operations++;
        }
    }
    return count_operations;
}

int delete_operation_block(struct ArrayOfBlocks *array, int index)
{
    if (array == NULL)
    {
        fprintf(stderr, "Null pointer exception\n");
        return EXIT_FAILURE;
    }
    else if (index >= array -> array_length)
    {
        fprintf(stderr, "Index is bigger than the length of the array\n");
        return EXIT_FAILURE;
    }

    char **block_array = array -> main_array[index].block_array;
    int block_length = array -> main_array[index].block_length;
    for (int i = 0; i < block_length; i++)
    {
        free(block_array[i]);
    }  
    free(block_array);

    return EXIT_SUCCESS;
}

int delete_operation(struct Block *block, int index)
{
    if (block == NULL)
    {
        printf("Null pointer exception\n");
        return EXIT_FAILURE;
    }
    else if (index >= block -> block_length)
    {
        printf("Index is bigger than the length of the array\n");
        return EXIT_FAILURE;
    }
    free(block -> block_array[index]);

    return EXIT_SUCCESS;
}