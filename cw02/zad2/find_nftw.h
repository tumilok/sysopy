#ifndef FIND_NFTW_H
#define FIND_NFTW_H

#include "utils.h"

void print_results_nftw(const char* file_path, const struct stat *stat);
int file_info(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
void nftw_exec();

#endif