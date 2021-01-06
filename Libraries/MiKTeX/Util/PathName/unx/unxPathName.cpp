/* unxPathName.cpp:

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

#include <unistd.h>

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

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

PathName& PathName::SetToCurrentDirectory()
{
  while (getcwd(GetData(), GetCapacity()) == nullptr)
  {
    if (errno == ERANGE)
    {
      Reserve(GetCapacity() * 2);
    }
    else
    {
      throw CRuntimeError("getcwd");
    }
  }
  return *this;
}

PathName& PathName::SetToTempDirectory()
{
  if (Helpers::GetEnvironmentString("TMPDIR", *this) && this->IsAbsolute())
  {
    return *this;
  }
#if defined(P_tmpdir)
  *this = P_tmpdir;
#else
  *this = "/tmp";
#endif
  return *this;
}

PathName& PathName::SetToTempFile(const PathName& directory)
{
  *this = directory;
  AppendComponent("mikXXXXXX");
  int fd = mkstemp(GetData());
  if (fd < 0)
  {
    throw CRuntimeError("mkstemp");
  }
  close(fd);
  return *this;
}

PathName PathName::GetMountPoint() const
{
  UNIMPLEMENTED();
}
