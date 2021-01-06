/* Helpers.cpp:

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

BEGIN_INTERNAL_NAMESPACE;

bool Helpers::IsPureAscii(const char* lpsz)
{
  for (; *lpsz != 0; ++lpsz)
  {
    if (static_cast<unsigned>(*lpsz) > 127)
    {
      return false;
    }
  }
  return true;
}

const char* Helpers::GetFileNameExtension(const char* path)
{
  const char* extension = nullptr;
  for (const char* lpsz = path; *lpsz != 0; ++lpsz)
  {
    if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
    {
      extension = nullptr;
    }
    else if (*lpsz == '.')
    {
      extension = lpsz;
    }
  }
  return extension;
}

void Helpers::RemoveDirectoryDelimiter(char* path)
{
  size_t l = strlen(path);
  if (l > 1 && PathNameUtil::IsDirectoryDelimiter(path[l - 1]))
  {
#if defined(MIKTEX_WINDOWS)
    if (path[l - 2] == PathNameUtil::DosVolumeDelimiter)
    {
      return;
    }
#endif
    path[l - 1] = 0;
  }
}

PathName Helpers::GetHomeDirectory()
{
  PathName result;
#if defined(MIKTEX_WINDOWS)
  string userProfile;
  if (Helpers::GetEnvironmentString("USERPROFILE", userProfile))
  {
    result = userProfile;
  }
  else
  {
    string homeDrive;
    string homePath;
    if (Helpers::GetEnvironmentString("HOMEDRIVE", homeDrive)
      && Helpers::GetEnvironmentString("HOMEPATH", homePath))
    {
      result = homeDrive + homePath;
    }
    else
    {
      result = "";
    }
  }
#else
  if (!Helpers::GetEnvironmentString("HOME", result))
  {
    result = "";
  }
#endif
  if (result.Empty())
  {
    throw Exception("Home directory is not defined.");
  }
  // TODO
  if (!Helpers::DirectoryExists(result))
  {
    throw Exception(fmt::format("Home directory {0} does not exist.", result));
  }
  return result;
}

bool Helpers::GetEnvironmentString(const string& name, PathName& path)
{
  string s;
  bool result = Helpers::GetEnvironmentString(name, s);
  if (result)
  {
    path = s;
  }
  return result;
}

END_INTERNAL_NAMESPACE;
