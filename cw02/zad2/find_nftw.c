#include "find_nftw.h"


int file_info(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if (ftwbuf -> level >= maxdepth)
    {
        return 0;
    }

    if (atime != -1)
    {
        if (check_time(atime, asign, sb -> st_atime) == 0)
        {
            return 0;
        }
    }

    if (mtime != -1)
    {
        if (check_time(mtime, msign, sb -> st_mtime) == 0)
        {
            return 0;
        }
    }

    print_results(fpath, sb);

    return 0;
}

void nftw_exec(char *path)
{
    nftw(path, file_info, 10, FTW_PHYS);
}