/* winPathName.cpp: path name utilities

   Copyright (C) 1996-2021 Christian Schenk

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

#include <Windows.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/PathName.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

PathName& PathName::SetToCurrentDirectory()
{
  wchar_t* buf = _wgetcwd(nullptr, MIKTEX_UTIL_PATHNAME_SIZE);
  if (buf == nullptr)
  {
    throw CRuntimeError("_wgetcwd");
  }
  *this = buf;
  free(buf);
  return *this;
}

PathName& PathName::SetToTempDirectory()
{
  for (const string& env : vector<string>{ "TMP", "TEMP", "USERPROFILE" })
  {
    if (Helpers::GetEnvironmentString(env, *this) && this->IsAbsolute())
    {
      return *this;
    }
  }
  wchar_t szTemp[MIKTEX_UTIL_PATHNAME_SIZE];
  unsigned long n = GetWindowsDirectoryW(szTemp, static_cast<DWORD>(MIKTEX_UTIL_PATHNAME_SIZE));
  if (n == 0)
  {
    throw WindowsError("GetWindowsDirectoryW");
  }
  if (n >= GetCapacity())
  {
    throw Unexpected("buf too small");
  }
  *this = szTemp;
  return *this;
}

PathName& PathName::SetToTempFile(const PathName& directory)
{
  wchar_t szTemp[MAX_PATH];
  UINT n = GetTempFileNameW(directory.ToWideCharString().c_str(), L"mik", 0, szTemp);
  if (n == 0)
  {
    throw WindowsError("GetTempFileNameW");
  }
  *this = szTemp;
  return *this;
}

PathName PathName::GetMountPoint() const
{
  wchar_t szDir[MIKTEX_UTIL_PATHNAME_SIZE];
  if (!GetVolumePathNameW(this->ToWideCharString().c_str(), szDir, MIKTEX_UTIL_PATHNAME_SIZE))
  {
    throw WindowsError("GetVolumePathNameW");
  }
  return PathName(szDir);
}

PathName& PathName::AppendAltDirectoryDelimiter()
{
  size_t l = GetLength();
  if (l == 0 || !PathNameUtil::IsDirectoryDelimiter(CharBuffer::operator[](l - 1)))
  {
    CharBuffer::Append(PathNameUtil::AltDirectoryDelimiter);
  }
  return *this;
}
