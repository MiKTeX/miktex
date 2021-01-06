/* winHelpers.cpp:

   Copyright (C) 2021 Christian Schenk

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

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

BEGIN_INTERNAL_NAMESPACE;

void Helpers::CanonicalizePathName(PathName& path)
{
  CharBuffer<wchar_t, MIKTEX_UTIL_PATHNAME_SIZE> fullPath;
  bool done = false;
  int rounds = 0;
  do
  {
    DWORD n = GetFullPathNameW(path.ToWideCharString().c_str(), fullPath.GetCapacity(), fullPath.GetData(), nullptr);
    if (n == 0)
    {
      throw WindowsError("GetFullPathNameW");
    }
    done = n < fullPath.GetCapacity();
    if (!done)
    {
      if (rounds > 0)
      {
        throw Unexpected("buf too small");
      }
      fullPath.Reserve(n);
    }
    rounds++;
  }
  while (!done);
  path = fullPath.GetData();
}

bool Helpers::GetEnvironmentString(const string& name, string& value)
{
  wchar_t* lpszValue = _wgetenv(StringUtil::UTF8ToWideChar(name).c_str());
  if (lpszValue == nullptr)
  {
    return false;
  }
  else
  {
    value = StringUtil::WideCharToUTF8(lpszValue);
    return true;
  }
}

static unsigned long GetFileAttributes_harmlessErrors[] = {
  ERROR_FILE_NOT_FOUND, // 2
  ERROR_PATH_NOT_FOUND, // 3
  ERROR_ACCESS_DENIED, // 5
  ERROR_NOT_READY, // 21
  ERROR_BAD_NETPATH, // 53
  ERROR_BAD_NET_NAME, // 67
  ERROR_INVALID_NAME, // 123
  ERROR_BAD_PATHNAME, // 161
};

bool Helpers::DirectoryExists(const PathName& path)
{
  unsigned long attributes = GetFileAttributesW(path.ToExtendedLengthPathName().ToWideCharString().c_str());
  if (attributes != INVALID_FILE_ATTRIBUTES)
  {
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
      return false;
    }
    return true;
  }
  unsigned long error = ::GetLastError();
  // TODO: range-based for loop
  for (int idx = 0; idx < sizeof(GetFileAttributes_harmlessErrors) / sizeof(GetFileAttributes_harmlessErrors[0]); ++idx)
  {
    if (error == GetFileAttributes_harmlessErrors[idx])
    {
      error = ERROR_SUCCESS;
      break;
    }
  }
  if (error != ERROR_SUCCESS)
  {
    throw WindowsError("GetFileAttributesW");
  }
  return false;
}

END_INTERNAL_NAMESPACE;
