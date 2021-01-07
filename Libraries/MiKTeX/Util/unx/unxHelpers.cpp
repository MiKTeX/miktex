/* unxHelpers.cpp:

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

#include <sys/stat.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

BEGIN_INTERNAL_NAMESPACE;

void Helpers::CanonicalizePathName(PathName& path)
{
  char* resolved = realpath(path.GetData(), nullptr);
  if (resolved == nullptr)
  {
    if (errno == ENOENT)
    {
      return;
    }
    throw CRuntimeError("realpath");
  }
  path = resolved;
  free(resolved);
}

bool Helpers::GetEnvironmentString(const string& name, string& value)
{
  const char* path = getenv(name.c_str());
  if (path == nullptr)
  {
    return false;
  }
  else
  {
    value = path;
    return true;
  }
}

bool Helpers::DirectoryExists(const PathName& path)
{
  struct stat statbuf;
  if (stat(path.GetData(), &statbuf) == 0)
  {
    if (S_ISDIR(statbuf.st_mode) == 0)
    {
      return false;
    }
    return true;
  }
  int error = errno;
  if (error != ENOENT)
  {
    throw CRuntimeError("stat");
  }
  return false;
}

END_INTERNAL_NAMESPACE;
