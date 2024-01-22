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

#include <string>

#include <cstring>
#include <cwchar>

MIKTEX_UTIL_BEGIN_NAMESPACE;

inline const char* StrChr(const char* s, int ch)
{
    return strchr(s, ch);
}

inline const wchar_t* StrChr(const wchar_t* s, wint_t ch)
{
    return wcschr(s, ch);
}

template<typename CharType> size_t StrLen(const CharType* s)
{
    const CharType* start = s;
    for (; *s != 0; ++s)
    {
    }
    return s - start;
}

template<> inline size_t StrLen<char>(const char* s)
{
    return strlen(s);
}

template<> inline size_t StrLen<wchar_t>(const wchar_t* s)
{
    return wcslen(s);
}

inline int CeeStringCompare(const char* s1, const char* s2, bool ignoreCase)
{
#if defined(_MSC_VER)
    return ignoreCase ? _stricmp(s1, s2) : strcmp(s1, s2);
#else
    return ignoreCase ? strcasecmp(s1, s2) : strcmp(s1, s2);
#endif
}

inline int CeeStringCompare(const char* s1, const char* s2, size_t n, bool ignoreCase)
{
#if defined(_MSC_VER)
    return ignoreCase ? _strnicmp(s1, s2, n) : strncmp(s1, s2, n);
#else
    return ignoreCase ? strncasecmp(s1, s2, n) : strncmp(s1, s2, n);
#endif
}

inline int CeeStringCompare(const char* s1, const char* s2)
{
    return CeeStringCompare(s1, s2, false);
}

inline int StringCompare(const std::string& s1, const std::string& s2, bool ignoreCase)
{
    return CeeStringCompare(s1.c_str(), s2.c_str(), ignoreCase);
}

inline int StringCompare(const std::string& s1, const std::string& s2)
{
    return StringCompare(s1, s2, false);
}

MIKTEX_UTIL_END_NAMESPACE;
