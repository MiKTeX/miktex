/**
 * @file miktex/Util/inliners.h
 * @author Christian Schenk
 * @brief Inline functions
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <cstring>
#include <cwchar>

MIKTEX_UTIL_BEGIN_NAMESPACE;

inline const char* StrChr(const char* lpsz, int ch)
{
    return strchr(lpsz, ch);
}

inline const wchar_t* StrChr(const wchar_t* lpsz, wint_t ch)
{
    return wcschr(lpsz, ch);
}

template<typename CharType> size_t StrLen(const CharType* lpsz)
{
    const CharType* start = lpsz;
    for (; *lpsz != 0; ++lpsz)
    {
    }
    return lpsz - start;
}

template<> inline size_t StrLen<char>(const char* lpsz)
{
    return strlen(lpsz);
}

template<> inline size_t StrLen<wchar_t>(const wchar_t* lpsz)
{
    return wcslen(lpsz);
}

inline int StringCompare(const char* lpsz1, const char* lpsz2, bool ignoreCase)
{
#if defined(_MSC_VER)
    return ignoreCase ? _stricmp(lpsz1, lpsz2) : strcmp(lpsz1, lpsz2);
#else
    return ignoreCase ? strcasecmp(lpsz1, lpsz2) : strcmp(lpsz1, lpsz2);
#endif
}

inline int StringCompare(const char* lpsz1, const char* lpsz2, size_t n, bool ignoreCase)
{
#if defined(_MSC_VER)
    return ignoreCase ? _strnicmp(lpsz1, lpsz2, n) : strncmp(lpsz1, lpsz2, n);
#else
    return ignoreCase ? strncasecmp(lpsz1, lpsz2, n) : strncmp(lpsz1, lpsz2, n);
#endif
}

inline int StringCompare(const char* lpsz1, const char* lpsz2)
{
    return StringCompare(lpsz1, lpsz2, false);
}

MIKTEX_UTIL_END_NAMESPACE;
