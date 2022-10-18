/**
 * @file c4plib.cpp
 * @author Christian Schenk
 * @brief C4P runtime routines
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#if defined(MIKTEX_WINDOWS)
#   include <Windows.h>
#   include <io.h>
#else
#   include <unistd.h>
#endif

#if defined(MIKTEX_TEXMF_SHARED)
#   define C4PEXPORT MIKTEXDLLEXPORT
#else
#   define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

#define C4P_BEGIN_NAMESPACE namespace C4P {
#define C4P_END_NAMESPACE }

C4P_BEGIN_NAMESPACE;

void C4P_text::DiscardLine()
{
    AssertValid();
    // FIXME: OS X
    while (!feof(file) && GetChar() != '\n')
    {
        ;
    }
}

inline bool C4P_text::IsTerminal()
{
    int fd = fileno(file);
    if (fd < 0)
    {
        MIKTEX_FATAL_CRT_ERROR_2("fileno", "path", path.ToString());
    }
#if defined(MIKTEX_WINDOWS)
    int x = _isatty(fd);
#else
    int x = isatty(fd);
#endif
#if 0
    if (errno != 0)
    {
        MIKTEX_FATAL_CRT_ERROR_2("isatty", "path", path.ToString());
    }
#endif
    return x != 0;
}

char C4P_text::GetChar()
{
    AssertValid();
    if (!IsPascalFileIO())
    {
        PascalFileIO(true);
        bufref() = GetC(file);
    }
    char ch = bufref();
    if (ch == '\n' && IsTerminal())
    {
        PascalFileIO(false);
    }
    else
    {
        int ch2 = GetC(file);
        if (ch2 != EOF)
        {
            bufref() = static_cast<char>(ch2);
        }
    }
    return ch;
}

C4P_integer C4P_text::GetInteger()
{
    AssertValid();
    while (!Eof())
    {
        int ch;
        do
        {
            ch = GetChar();
        } while (!Eof() && !isdigit(ch) && ch != '-' && ch != '+');
        int sign = (ch == '-' ? -1 : 1);
        if ((ch == '+' || ch == '-') && !Eof())
        {
            ch = GetChar();
        }
        if (isdigit(ch))
        {
            C4P_integer result = 0;
            while (isdigit(ch))
            {
                result *= 10;
                result += (ch - '0');
                if (Eof())
                {
                    break;
                }
                ch = GetChar();
            }
            return result * sign;
        }
    }
    return 0;
}

C4P_real C4P_text::GetReal()
{
    MIKTEX_UNIMPLEMENTED();
}

bool FileRoot::Open(const PathName& path, FileMode mode, FileAccess access, bool text, bool mustExist)
{
    this->path = path;
    FILE* file;
    shared_ptr<Session> session = MIKTEX_SESSION();
    if (mustExist)
    {
        file = session->OpenFile(path, mode, access, text);
    }
    else
    {
        file = session->TryOpenFile(path, mode, access, text);
        if (file == nullptr)
        {
            return false;
        }
    }
    Attach(file, true);
    return true;
}

C4PCEEAPI(C4P_integer) Round(double r)
{
    if (r > INT_MAX)
    {
        return INT_MAX;
    }
    else if (r < -INT_MAX)
    {
        return -INT_MAX;
    }
    else if (r >= 0.0)
    {
        return static_cast<C4P_integer>(r + 0.5);
    }
    else
    {
        return static_cast<C4P_integer>(r - 0.5);
    }
}

C4P_END_NAMESPACE;
