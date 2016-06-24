/* inliners.h:

   Copyright (C) 1996-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#include "Session/SessionImpl.h"

BEGIN_INTERNAL_NAMESPACE;

#define CTYPE_FACET \
  std::use_facet<std::ctype<char>>(SessionImpl::GetDefaultLocale())

#define CTYPE_FACET_W \
  std::use_facet<std::ctype<wchar_t>>(SessionImpl::GetDefaultLocale())

inline char ToLower(char ch)
{
  MIKTEX_ASSERT(static_cast<unsigned>(ch) < 128);
  if (ch >= 'A' && ch <= 'Z')
  {
    ch = ch - 'A' + 'a';
  }
  return ch;
}

inline wchar_t ToLower(wchar_t ch)
{
  if (static_cast<unsigned>(ch) < 128)
  {
    if (ch >= L'A' && ch <= L'Z')
    {
      ch = ch - L'A' + L'a';
    }
  }
  else
  {
    ch = CTYPE_FACET_W.tolower(ch);
  }
  return ch;
}

inline char ToUpper(char ch)
{
  MIKTEX_ASSERT(static_cast<unsigned>(ch) < 128);
  if (ch >= 'a' && ch <= 'z')
  {
    ch = ch - 'a' + 'A';
  }
  return ch;
}

inline wchar_t ToUpper(wchar_t ch)
{
  if (static_cast<unsigned>(ch) < 128)
  {
    if (ch >= L'a' && ch <= L'z')
    {
      ch = ch - L'a' + L'A';
    }
  }
  else
  {
    ch = CTYPE_FACET_W.toupper(ch);
  }
  return ch;
}

#if defined(MIKTEX_WINDOWS)
inline bool IsDriveLetter(char ch)
{
  return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z';
}
#endif

inline bool IsAlpha(char ch)
{
  return CTYPE_FACET.is(std::ctype<char>::alpha, ch);
}

inline bool IsDigit(char ch)
{
  return CTYPE_FACET.is(std::ctype<char>::digit, ch);
}

inline bool IsAlNum(char ch)
{
  return CTYPE_FACET.is(std::ctype<char>::alnum, ch);
}

inline bool IsSpace(char ch)
{
  return CTYPE_FACET.is(std::ctype<char>::space, ch);
}

inline void SkipSpace(const char * & lpsz)
{
  while (*lpsz != 0 && IsSpace(*lpsz))
  {
    ++lpsz;
  }
}

inline void SkipSpace(char * & lpsz)
{
  while (*lpsz != 0 && IsSpace(*lpsz))
  {
    ++lpsz;
  }
}

inline void SkipAlpha(const char * & lpsz)
{
  while (*lpsz != 0 && IsAlpha(*lpsz))
  {
    ++lpsz;
  }
}

inline void SkipNonDigit(const char * & lpsz)
{
  while (*lpsz != 0 && !IsDigit(*lpsz))
  {
    ++lpsz;
  }
}

END_INTERNAL_NAMESPACE;
