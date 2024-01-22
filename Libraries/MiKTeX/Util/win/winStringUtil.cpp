/**
 * @file winStringUtil.cpp
 * @author Christian Schenk
 * @brief StringUtil class (Windows)
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <Windows.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/CharBuffer.h"
#include "miktex/Util/StringUtil.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

static wchar_t* AnsiCeeToWideCharCee(const char* source, wchar_t* dest, size_t destSize)
{
    if (*source == 0)
    {
        // TODO: MIKTEX_ASSERT(destSize > 0);
        *dest = 0;
        return dest;
    }
    int n = MultiByteToWideChar(CP_ACP, 0, source, -1, dest, static_cast<int>(destSize));
    if (n == 0)
    {
        throw exception("Conversion from narrow string (ANSI) to wide character string did not succeed.");
    }
    if (n < 0)
    {
        FATAL_ERROR();
    }
    return dest;
}

string StringUtil::AnsiToUTF8(const string& ansi)
{
    CharBuffer<wchar_t, 512> buf(ansi.length() + 1);
    return StringUtil::WideCharToUTF8(AnsiCeeToWideCharCee(ansi.c_str(), buf.GetData(), buf.GetCapacity()));
}
