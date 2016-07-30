/* unxDirectory.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Directory.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;

PathName Directory::GetCurrent()
{
  PathName cd;
  cd.SetToCurrentDirectory();
  return cd;
}

void Directory::SetCurrent(const PathName & path)
{
  if (chdir(path.Get()) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("chdir", "path", path.ToString());
  }
}

bool Directory::Exists(const PathName & path)
{
  struct stat statbuf;
  if (stat(path.Get(), &statbuf) == 0)
  {
    if (S_ISDIR(statbuf.st_mode) == 0)
    {
      SessionImpl::GetSession()->trace_access->WriteFormattedLine("core", T_("%s is not a directory"), Q_(path));
      return false;
    }
    return true;
  }
  int error = errno;
  if (error != ENOENT)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", path.ToString());
  }
  return false;
}

void Directory::Delete(const PathName & path)
{
  SessionImpl::GetSession()->trace_files->WriteFormattedLine("core", T_("deleting directory %s"), Q_(path));
  if (rmdir(path.Get()) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("rmdir", "path", path.ToString());
  }
}

void Directory::SetTimes(const PathName & path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  File::SetTimes(path, creationTime, lastAccessTime, lastWriteTime);
}

void Directory::Move(const PathName & source, const PathName & dest)
{
  File::Move(source, dest);
}
