#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>


const char *format;
int atime, mtime;
int maxdepth;
char asign, msign;

int check_time(int count, char sign, time_t time_from_file);
void print_results(const char* fpath, const struct stat *sb);

#endif