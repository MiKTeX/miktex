/**
 * @file j2000.cpp
 * @author Christian Schenk
 * @brief Compute number of days since Jan 1, 2000
 *
 * @copyright Copyright © 2000-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <cstdio>
#include <ctime>
#include <sys/stat.h>

int main(int argc, const char** argv)
{
    time_t tQuery;
    const time_t t2000 = 946681200;
    if (argc > 1)
    {
        struct stat statbuf;
        if (stat(argv[1], &statbuf) != 0)
        {
            return 0;
        }
        tQuery = statbuf.st_mtime;
    }
    else
    {
        time(&tQuery);
    }
    int days = static_cast<int>((tQuery - t2000) / (60 * 60 * 24));
    printf("%d\n", days);
    return 0;
}
