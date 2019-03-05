/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2001-2019 Christian Schenk

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

#if !defined(D02AFD831E4F4B9E8394D2B294DF4582)
#define D02AFD831E4F4B9E8394D2B294DF4582

#include <ctime>

#include <string>

#include <miktex/Core/PathName>
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

MPM_INTERNAL_BEGIN_NAMESPACE;

constexpr const char* MPM_AGENT = "MPM/" MIKTEX_COMPONENT_VERSION_STR;

// the trailing slash should not be removed
constexpr const char* TEXMF_PREFIX_DIRECTORY = "texmf" MIKTEX_PATH_DIRECTORY_DELIMITER_STRING;

struct hash_path
{
public:
  std::size_t operator()(const std::string& str) const
  {
    return MiKTeX::Core::PathName(str).GetHash();
  }
};

struct equal_path
{
public:
  bool operator()(const std::string& str1, const std::string& str2) const
  {
    return MiKTeX::Core::PathName::Compare(str1, str2) == 0;
  }
};

inline void DbgView(const std::string& s)
{
#if defined(_WIN32)
  OutputDebugStringW(UW_("MiKTeX MPM: " + s));
#endif
}

inline bool StripPrefix(const std::string& str, const char* lpszPrefix, std::string& result)
{
  std::size_t n = MiKTeX::Util::StrLen(lpszPrefix);
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

bool IsUrl(const std::string& url);

std::string MakeUrl(const std::string& base, const std::string& rel);

MPM_INTERNAL_END_NAMESPACE;

#endif
