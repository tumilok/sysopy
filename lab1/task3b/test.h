#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <time.h>
#include "comparison.h"

void compare_pair(struct ArrayOfBlocks *array, char *pair);
void remove_block(struct ArrayOfBlocks *array, int index);
void remove_operation(struct Block *block, int index);
void read_arguments(int argc, char *argv[]);

#endif
