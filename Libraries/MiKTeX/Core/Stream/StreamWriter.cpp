/**
 * @file Stream/StreamWriter.cpp
 * @author Christian Schenk
 * @brief StreamWriter implementation
 *
 * @copyright Copyright © 1996-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <miktex/Core/StreamWriter>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

StreamWriter::StreamWriter(const PathName& path) :
    stream(File::Open(path, FileMode::Create, FileAccess::Write))
{
}

StreamWriter::~StreamWriter() noexcept
{
    try
    {
        Close();
    }
    catch (const exception&)
    {
    }
}

void StreamWriter::Close()
{
    stream.Close();
}

inline int FPutC(int ch, FILE* stream)
{
    int chWritten = fputc(ch, stream);
    if (chWritten == EOF)
    {
        MIKTEX_FATAL_CRT_ERROR("fputc");
    }
    return chWritten;
}

inline void FPutS(const char* lpsz, FILE* stream)
{
    int ok = fputs(lpsz, stream);
    if (ok < 0)
    {
        MIKTEX_FATAL_CRT_ERROR("fputs");
    }
}

void StreamWriter::Write(char ch)
{
    FPutC(ch, stream.GetFile());
}

void StreamWriter::Write(const string& s)
{
    FPutS(s.c_str(), stream.GetFile());
}

void StreamWriter::WriteLine(const string& s)
{
    Write(s);
    WriteLine();
}

void StreamWriter::WriteLine()
{
    FPutC('\n', stream.GetFile());
}
