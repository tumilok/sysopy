#include "test.h"

void compare_pair(struct ArrayOfBlocks *array, char *pair)
{
    int i = 0;
    while (pair[i] != ':' && i < strlen(pair))
    {
        i++;
    }

    char *fname1 = calloc(i, sizeof(char));
    strncpy(fname1, pair, i);
    char *fname2 = calloc(strlen(pair) - i + 1, sizeof(char));
    strncpy(fname2, pair + i + 1, strlen(pair) - i + 1);

    compare_files(fname1, fname2);

    clock_t start, stop;
    start = clock();
    add_operation_block(array);
    stop = clock();
    printf("adding operation block time: %f\n", (((double) (stop - start)) / CLOCKS_PER_SEC));
}

void remove_block(struct ArrayOfBlocks *array, int index)
{
    clock_t start, stop;
    start = clock();
    delete_operation_block(array, index);
    stop = clock();
    printf("removing block time: %f\n", (((double) (stop - start)) / CLOCKS_PER_SEC));
}

void remove_operation(struct Block *block, int index)
{
    clock_t start, stop;
    start = clock();
    delete_operation(block, index);
    stop = clock();
    printf("removing operation time: %f\n", (((double) (stop - start)) / CLOCKS_PER_SEC));
}

void read_arguments(int argc, char *argv[])
{
    struct ArrayOfBlocks array = init_main_array();
    bool is_argument = false;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "compare_pair") == 0)
        {
            is_argument = true;
            continue;
        }

        if (strcmp(argv[i], "remove_block") == 0)
        {
            is_argument = false;
            remove_block(&array, (int)(argv[i + 1][0] - '0'));
            i++;
            continue;
        }

        if (strcmp(argv[i], "remove_operation") == 0)
        {
            is_argument = false;
            struct Block block = array.main_array[(int)(argv[i + 1][0] - '0')];
            remove_operation(&block, (int)(argv[i + 1][0] - '0'));
            i+=2;
            continue;
        }

        if (is_argument)
        {
            compare_pair(&array, argv[i]);
            continue;
        }
    }
}