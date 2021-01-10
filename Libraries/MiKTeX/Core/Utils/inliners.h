/* inliners.h:

   Copyright (C) 1996-2021 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

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
