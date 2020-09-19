/* unx.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <sys/stat.h>

#if defined(HAVE_SYS_STATVFS_H)
#  include <sys/statvfs.h>
#endif

#include <miktex/Core/Directory>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

MIKTEXINTERNALFUNC(bool) FileIsOnROMedia(const char* lpszPath)
{
#if defined(HAVE_STATVFS)
  struct statvfs buf;
  if (statvfs(lpszPath, &buf) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("statvfs", "path", lpszPath);
  }
  return (buf.f_flag & ST_RDONLY) != 0;
#else
#warning Unimplemented : FileIsOnROMedia()
  UNUSED_ALWAYS(lpszPath);
  return false;
#endif
}

MIKTEXSTATICFUNC(void) CreateDirectoryPathWithMode(const PathName& path, mode_t mode)
{
  if (!path.IsAbsolute())
  {
    PathName absPath(path);
    absPath.MakeFullyQualified();
    // RECURSION
    CreateDirectoryPathWithMode(absPath, mode);
  }

  // do nothing, if the directory already exists
  if (Directory::Exists(path))
  {
    return;
  }

  // create the parent directory
  PathName pathParent = path.GetDirectoryName();
  RemoveDirectoryDelimiter(pathParent.GetData());
  // RECURSION
  CreateDirectoryPathWithMode(pathParent, mode);

  // we're done, if we have no file name
  if (path.GetFileName().Empty())
  {
    return;
  }

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  if (session != nullptr)
  {
    session->trace_config->WriteLine("core", fmt::format(T_("creating directory {0}..."), Q_(path)));
  }

  // create the directory itself
  if (mkdir(path.GetData(), mode) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("mkdir", "path", path.ToString());
  }
}

MIKTEXINTERNALFUNC(void) CreateDirectoryPath(const PathName& path)
{
  CreateDirectoryPathWithMode(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

#if 0
MIKTEXINTERNALFUNC(void) CreateDirectoryPathForEveryone(const char* lpszPath)
{
  CreateDirectoryPathWithMode(lpszPath, S_IRWXU | S_IRWXG | S_IRWXO);
}
#endif
