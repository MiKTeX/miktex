/**
 * @file padout.cpp
 * @author Christian Schenk
 * @brief Pad out a binary file
 *
 * @copyright Copyright © 2007-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>

using namespace std;

int main(int argc, const char** argv)
{
    if (argc != 2)
    {
        cerr << "Usage: padout FILE" << '\n';
        return 1;
    }

    FILE* file = fopen(argv[1], "ab");

    if (file == nullptr)
    {
        cerr << "cannot open " << argv[1] << '\n';
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        cerr << "seek error" << '\n';
        return 1;
    }

    long size = ftell(file);

    if (size < 0)
    {
        cerr << "I/O error" << '\n';
        return 1;
    }

    srand(static_cast<unsigned>(time(nullptr)));

    for (; (size % 512) != 0; ++size)
    {
        int byte = rand() & 0xff;
        if (fputc(byte, file) == EOF)
        {
            cerr << "I/O error" << '\n';
            return 1;
        }
        ++size;
    }

    fclose(file);

    return 0;
}
