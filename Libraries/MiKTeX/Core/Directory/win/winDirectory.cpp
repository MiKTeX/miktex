/* winDirectory.cpp:

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

#include <miktex/Core/Directory>
#include <miktex/Core/win/winAutoResource>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

PathName Directory::GetCurrent()
{
  PathName cd;
  cd.SetToCurrentDirectory();
  return cd;
}

void Directory::SetCurrent(const PathName& path)
{
  if (_wchdir(UW_(path.GetData())) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("_wchdir", "path", path.ToString());
  }
}

static unsigned long GetFileAttributes_harmlessErrors[] = {
  ERROR_FILE_NOT_FOUND, // 2
  ERROR_PATH_NOT_FOUND, // 3
  ERROR_NOT_READY, // 21
  ERROR_BAD_NETPATH, // 53
  ERROR_BAD_NET_NAME, // 67
  ERROR_INVALID_NAME, // 123
  ERROR_BAD_PATHNAME, // 161
};

bool Directory::Exists(const PathName& path)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  unsigned long attributes = GetFileAttributesW(path.ToExtendedLengthPathName().ToWideCharString().c_str());
  if (attributes != INVALID_FILE_ATTRIBUTES)
  {
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
      if (session != nullptr)
      {
        session->trace_access->WriteLine("core", fmt::format(T_("{0} is not a directory"), Q_(path)));
      }
      return false;
    }
    if (session != nullptr)
    {
      session->trace_access->WriteLine("core", fmt::format(T_("accessing directory {0}: OK"), Q_(path)));
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
    MIKTEX_FATAL_WINDOWS_ERROR_3("GetFileAttributesW",
      T_("MiKTeX cannot retrieve attributes for the directory '{path}'."),
      "path", path.ToDisplayString());
  }
  if (session != nullptr)
  {
    session->trace_access->WriteLine("core", fmt::format(T_("accessing directory {0}: NOK"), Q_(path)));
  }
  return false;
}

void Directory::Delete(const PathName& path)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteLine("core", fmt::format(T_("deleting directory {0}"), Q_(path)));
  }
  if (!RemoveDirectoryW(path.ToExtendedLengthPathName().ToWideCharString().c_str()))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("RemoveDirectoryW", "path", path.ToString());
  }
}

void Directory::SetTimes(const PathName& path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  HANDLE h = CreateFileW(path.ToExtendedLengthPathName().ToWideCharString().c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (h == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
  }
  AutoHANDLE autoClose(h);
  SetTimesInternal(h, creationTime, lastAccessTime, lastWriteTime);
}

void Directory::Move(const PathName& source, const PathName& dest)
{
  File::Move(source, dest);
}
