/**
 * @file inliners.h
 * @author Christian Schenk
 * @brief Inline functions
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include "internal.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

inline char ToLowerAscii(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        ch = ch - 'A' + 'a';
    }
    return ch;
}

inline char ToUpperAscii(char ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        ch = ch - 'a' + 'A';
    }
    return ch;
}

inline bool IsAlphaAscii(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

inline bool IsDecimalDigitAscii(char ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool IsAlphaNumericAScii(char ch)
{
    return IsAlphaAscii(ch) || IsDecimalDigitAscii(ch);
}

inline bool IsWhitespaceAscii(char ch)
{
    return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\n';
}

inline void SkipWhitespaceAscii(const char*& lpsz)
{
    while (*lpsz != 0 && IsWhitespaceAscii(*lpsz))
    {
        ++lpsz;
    }
}

inline void SkipWhitespaceAscii(char*& lpsz)
{
    while (*lpsz != 0 && IsWhitespaceAscii(*lpsz))
    {
        ++lpsz;
    }
}

inline void SkipAlphaAscii(const char*& lpsz)
{
    while (*lpsz != 0 && IsAlphaAscii(*lpsz))
    {
        ++lpsz;
    }
}

inline void SkipNonDecimalDigitAscii(const char*& lpsz)
{
    while (*lpsz != 0 && !IsDecimalDigitAscii(*lpsz))
    {
        ++lpsz;
    }
}

inline size_t SkipNonDecimalDigitAscii(const std::string& s, size_t pos = 0)
{
    while (pos < s.length() && !IsDecimalDigitAscii(s[pos]))
    {
        ++pos;
    }
    return pos;
}

CORE_INTERNAL_END_NAMESPACE;
