/* unxemu.cpp:

   Copyright (C) 2007-2017 Christian Schenk

   This file is part of the MiKTeX UNXEMU Library.

   The MiKTeX UNXEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UNXEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UNXEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "internal.h"

#include <errno.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

#include <WinSock2.h>

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

struct DIR_
{
  unique_ptr<DirectoryLister> pLister;
  struct dirent direntry;
  PathName path;
  DIR_(const char * lpszPath) :
    path(lpszPath),
    pLister(DirectoryLister::Open(lpszPath))
  {
  }
};

struct WDIR_
{
  unique_ptr<DirectoryLister> pLister;
  struct wdirent direntry;
  PathName path;
  WDIR_(const wchar_t * lpszPath) :
    path(lpszPath),
    pLister(DirectoryLister::Open(lpszPath))
  {
  }
};

MIKTEXUNXCEEAPI(int) closedir(DIR * pDir)
{
  C_FUNC_BEGIN();
  delete pDir;
  return 0;
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(int) wclosedir(WDIR * pDir)
{
  C_FUNC_BEGIN();
  delete pDir;
  return 0;
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(DIR *) opendir(const char * lpszPath)
{
  C_FUNC_BEGIN();
  if (!Directory::Exists(lpszPath))
  {
    errno = ENOENT;
    return nullptr;
  }
  return new DIR_(lpszPath);
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(WDIR *) wopendir(const wchar_t * lpszPath)
{
  C_FUNC_BEGIN();
  if (!Directory::Exists(lpszPath))
  {
    errno = ENOENT;
    return nullptr;
  }
  return new WDIR_(lpszPath);
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(struct dirent *) readdir(DIR * pDir)
{
  C_FUNC_BEGIN();
  DirectoryEntry directoryEntry;
  if (!pDir->pLister->GetNext(directoryEntry))
  {
    return nullptr;
  }
  StringUtil::CopyString(pDir->direntry.d_name, sizeof(pDir->direntry.d_name) / sizeof(pDir->direntry.d_name[0]), directoryEntry.name.c_str());
  return &pDir->direntry;
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(struct wdirent *)
wreaddir(WDIR * pDir)
{
  C_FUNC_BEGIN();
  DirectoryEntry directoryEntry;
  if (!pDir->pLister->GetNext(directoryEntry))
  {
    return nullptr;
  }
  StringUtil::CopyString(pDir->direntry.d_name, sizeof(pDir->direntry.d_name) / sizeof(pDir->direntry.d_name[0]), directoryEntry.wname.c_str());
  return &pDir->direntry;
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(void) rewinddir(DIR * pDir)
{
  C_FUNC_BEGIN();
  pDir->pLister->Close();
  pDir->pLister = DirectoryLister::Open(pDir->path);
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(void) wrewinddir(WDIR * pDir)
{
  C_FUNC_BEGIN();
  pDir->pLister->Close();
  pDir->pLister = DirectoryLister::Open(pDir->path);
  C_FUNC_END();
}

MIKTEXUNXCEEAPI(int) miktex_strncasecmp(const char * lpsz1, const char * lpsz2, size_t n)
{
  return MiKTeX::Util::StringCompare(lpsz1, lpsz2, n, true);
}

MIKTEXUNXCEEAPI(int) miktex_gettimeofday(struct timeval * ptv, void * pNull)
{
  MIKTEX_ASSERT(pNull == nullptr);
  MIKTEX_ASSERT(ptv != nullptr);
  SYSTEMTIME systemTime;
  GetSystemTime(&systemTime);
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_year = systemTime.wYear;
  tm.tm_mon = systemTime.wMonth - 1;
  tm.tm_mday = systemTime.wDay;
  tm.tm_hour = systemTime.wHour;
  tm.tm_min = systemTime.wMinute;
  tm.tm_sec = systemTime.wSecond;
  ptv->tv_sec = static_cast<long>(mktime(&tm));
  ptv->tv_usec = systemTime.wMilliseconds;
  return 0;
}

// derived from glibc 2.3.6 libc/sysdeps/posix/tempname.c
// Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
MIKTEXUNXCEEAPI(int) miktex_mkstemp(char* tmpl)
{
  size_t len = strlen(tmpl);
  if (len < 6 || strcmp(&tmpl[len - 6], "XXXXXX") != 0)
  {
    // TODO
    return -1;
  }
  char* const XXXXXX = &tmpl[len - 6];
  static const char letters[] = "ABCDEFGHIJKLMnopqrstuvwxyz0123456789";
  const int lettercount = sizeof(letters) - 1;
  uint64_t value = time(nullptr) ^ GetCurrentProcessId();
  for (int rounds = 1000; rounds> 0; rounds--, value += 7777)
  {
    uint64_t v = value;
    for (char *x = XXXXXX; *x != 0; ++x, v /= lettercount)
    {
      *x = letters[v % lettercount];
    }
    int fd = _open(tmpl, _O_CREAT | _O_EXCL | _O_RDWR, _S_IREAD | _S_IWRITE);
    if (fd >= 0)
    {
      return fd;
    }
    else if (fd != EEXIST)
    {
      // TODO
      return -1;
    }
  }
  // TODO
  return -1;
}
