#ifndef FIND_STAT_H
#define FIND_STAT_H

#include "utils.h"

int check_conditions(struct stat* dir_stat);
void find_dir(char *fpath, int depth);
void stat_exec();

#endif