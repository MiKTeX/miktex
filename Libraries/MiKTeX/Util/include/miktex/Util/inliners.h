/* miktex/Util/inliners.h:                              -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(C0ED03A793AF458292B01CFA00749B67)
#define C0ED03A793AF458292B01CFA00749B67

#include "config.h"

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

inline int StringCompare(const char*  lpsz1, const char* lpsz2, size_t n, bool ignoreCase)
{
  // TODO: MIKTEX_ASSERT(!ignoreCase || MiKTeX::Util::Utils::IsPureAscii(lpsz1) && MiKTeX::Util::Utils::IsPureAscii(lpsz2));
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

#endif
