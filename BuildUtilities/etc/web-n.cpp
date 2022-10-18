/**
 * @file web-n.cpp
 * @author Christian Schenk
 * @brief Find web section numbers
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

#define SEP1 "\
%% _____________________________________________________________________________\n\
%%\n\
%% [%d.%d] %s\n\
%% _____________________________________________________________________________\n"

#define SEP2 "\
%% _____________________________________________________________________________\n\
%%\n\
%% [%d.%d]\n\
%% _____________________________________________________________________________\n"


int main()
{
    char szLine[2048];
    int iSecNum = 0;
    int iChapNum = 0;
    while (fgets(szLine, sizeof(szLine), stdin))
    {
        if (szLine[0] == '@'
            && (szLine[1] == '*' || szLine[1] == ' ' || szLine[1] == '\n'))
        {
            ++iSecNum;
            fputc('\n', stdout);
            if (szLine[1] == '*')
            {
                ++iChapNum;
                char* lpsz = &szLine[2];
                while (*lpsz != 0 && isspace(*lpsz))
                {
                    ++lpsz;
                }
                string strTitle;
                while (*lpsz != 0 && *lpsz != '.')
                {
                    strTitle += *lpsz;
                    ++lpsz;
                }
                fprintf(stdout, SEP1, iChapNum, iSecNum, strTitle.c_str());
            }
            else
            {
                fprintf(stdout, SEP2, iChapNum, iSecNum);
            }
            fputc('\n', stdout);
        }
        fputs(szLine, stdout);
    }
    return 0;
}
