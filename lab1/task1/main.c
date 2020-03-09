#include "comparison.h"

int main()
{
    const char *fname1 = "a.txt";
    const char *fname2 = "b.txt";
    const char *fname3 = "c.txt";

    struct ArrayOfBlocks array = init_main_array();
    compare_files(fname1, fname2);
    printf("Index of block: %d\n", add_operation_block(&array));
    compare_files(fname2, fname3);
    printf("Index of block: %d\n", add_operation_block(&array));
    printf("Number of block operations: %d\n", get_block_length(&array.main_array[0]));
    printf("Number of block operations: %d\n", get_block_length(&array.main_array[1]));
    print_arrays(&array);

    delete_operation(&array.main_array[0], 0);
    printf("Number of block operations: %d\n", get_block_length(&array.main_array[0]));
    print_arrays(&array);

    delete_operation_block(&array, 1);
    delete_operation_block(&array, 0);
    free(array.main_array);

    return 0;
}