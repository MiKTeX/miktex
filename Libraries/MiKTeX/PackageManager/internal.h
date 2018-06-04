/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <string>

#include <miktex/Core/Paths>
#include <miktex/Core/Session>

#include "text.h"

#if !defined(UNUSED)
#  if !defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) static_cast<void>(x)
#  endif
#endif

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

BEGIN_INTERNAL_NAMESPACE;

const char* const MPM_AGENT = "MPM/" MIKTEX_COMPONENT_VERSION_STR;

// the trailing slash should not be removed
#define TEXMF_PREFIX_DIRECTORY \
  "texmf" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING

inline void DbgView(const std::string& s)
{
#if defined(_WIN32)
  OutputDebugStringW(UW_("MiKTeX MPM: " + s));
#endif
}

inline bool StripPrefix(const std::string& str, const char* lpszPrefix, std::string& result)
{
  size_t n = MiKTeX::Util::StrLen(lpszPrefix);
  if (MiKTeX::Core::PathName::Compare(str.c_str(), lpszPrefix, n) != 0)
  {
    return false;
  }
  result = str.c_str() + n;
  return true;
}

#if defined(StrCmp)
#  undef StrCmp
#endif

inline int StrCmp(const char* lpsz1, const char* lpsz2)
{
  return strcmp(lpsz1, lpsz2);
}

inline int StrCmp(const wchar_t* lpsz1, const wchar_t* lpsz2)
{
  return wcscmp(lpsz1, lpsz2);
}

inline int FPutS(const char* lpsz, FILE* stream)
{
  int n = fputs(lpsz, stream);
  if (n < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fputs");
  }
  return n;
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

END_INTERNAL_NAMESPACE;

