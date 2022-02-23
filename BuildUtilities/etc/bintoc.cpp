/**
 * @file bintoc.cpp
 * @author Christian Schenk
 * @brief Make a C char array from a binary file
 *
 * @copyright Copyright © 2000-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include <iomanip>
#include <iostream>
#include <string>

#if (defined(_MSC_VER) && defined(_WIN32)) || defined(__CYGWIN__)
#  include <io.h>
#endif

size_t totalBytes;
size_t remainingBytesOnLine;

using namespace std;

void OpenArray(const string& arrayName)
{
    cout << "unsigned char const " << arrayName << "[] = {";
    totalBytes = 0;
    remainingBytesOnLine = 0;
}

void CloseArray()
{
    cout << "\n};\n";
}

void CloseLine()
{
    cout << "\n";
}

void AddByte(unsigned char byt)
{
    if (remainingBytesOnLine == 0)
    {
        CloseLine();
        remainingBytesOnLine = 8;
        cout << " ";
    }
    cout << " 0x" << setw(2) << setfill('0') << hex << static_cast<unsigned>(byt) << ",";
    ++totalBytes;
    --remainingBytesOnLine;
}

unsigned char buf[4096 * 16];

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3)
    {
        return 1;
    }

    bool appendNul = argc > 2 && strcmp(argv[2], "true") == 0;

#if defined(_MSC_VER) && defined(_WIN32)
    if (_setmode(_fileno(stdin), _O_BINARY) == -1)
    {
        return 1;
    }
#elif defined(__CYGWIN__)
    if (setmode(fileno(stdin), O_BINARY) == -1)
    {
        return 1;
    }
#endif

    OpenArray(argv[1]);

    size_t n;

    while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
    {
        for (size_t i = 0; i < n; ++i)
        {
            AddByte(buf[i]);
        }
    }

    if (appendNul)
    {
        AddByte(0);
    }

    CloseArray();

    return 0;
}
