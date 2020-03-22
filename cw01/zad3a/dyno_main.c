#include <stdbool.h>
#include <time.h>
#include "comparison.h"

#include <dlfcn.h>

void *dl_handle;

typedef void *(*arbitrary)();

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

    arbitrary fcompare_files;
    *(void **)(&fcompare_files) = dlsym(dl_handle, "compare_files");
    fcompare_files(fname1, fname2);

    clock_t start, stop;
    start = clock();

    arbitrary fadd_operation_block;
    *(void **)(&fadd_operation_block) = dlsym(dl_handle, "add_operation_block");
    fadd_operation_block(array);

    stop = clock();
    printf("adding operation block time: %f\n", (((double) (stop - start)) / CLOCKS_PER_SEC));
}

void remove_block(struct ArrayOfBlocks *array, int index)
{
    clock_t start, stop;
    start = clock();

    arbitrary fdelete_operation_block;
    *(void **)(&fdelete_operation_block) = dlsym(dl_handle, "delete_operation_block");   
    fdelete_operation_block(array, index);

    stop = clock();
    printf("removing block time: %f\n", (((double) (stop - start)) / CLOCKS_PER_SEC));
}

void remove_operation(struct Block *block, int index)
{
    arbitrary fdelete_operation;
    *(void **)(&fdelete_operation) = dlsym(dl_handle, "fdelete_operation");  
    fdelete_operation(block, index);
}

void read_arguments(int argc, char *argv[])
{
    struct ArrayOfBlocks array;
    array.array_length = 0;

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

int main(int argc, char *argv[])
{
    dl_handle = dlopen("./libcomparison.so", RTLD_LAZY);
    if (!dl_handle) {
        printf("!!! %s\n", dlerror());
        return 0;
    }

    read_arguments(argc, argv);

    return 0;
}