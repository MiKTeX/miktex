/* winDirectoryLister.cpp: directory lister

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

#include "miktex/Core/DirectoryLister.h"
#include "miktex/Core/win/WindowsVersion"

#include "winDirectoryLister.h"

using namespace MiKTeX::Core;
using namespace std;

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName & directory)
{
  return make_unique<winDirectoryLister>(directory, nullptr, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName & directory, const char * lpszPattern)
{
  return make_unique<winDirectoryLister>(directory, lpszPattern, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName & directory, const char * lpszPattern, int options)
{
  return make_unique<winDirectoryLister>(directory, lpszPattern, options);
}

winDirectoryLister::winDirectoryLister(const PathName & directory, const char * lpszPattern, int options) :
  directory(directory),
  pattern(lpszPattern == nullptr ? "" : lpszPattern),
  options(options)
{
}

winDirectoryLister::~winDirectoryLister()
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void winDirectoryLister::Close()
{
  HANDLE handle = this->handle;
  if (handle == INVALID_HANDLE_VALUE)
  {
    return;
  }
  this->handle = INVALID_HANDLE_VALUE;
  if (!FindClose(handle))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("FindClose");
  }
}

bool winDirectoryLister::GetNext(DirectoryEntry & direntry)
{
  DirectoryEntry2 direntry2;
  if (!GetNext(direntry2))
  {
    return false;
  }
  direntry.name = direntry2.name;
  direntry.wname = direntry2.wname;
  direntry.isDirectory = direntry2.isDirectory;
  return true;
}

inline bool IsDotDirectory(const wchar_t * lpszDirectory)
{
  if (lpszDirectory[0] != L'.')
  {
    return false;
  }
  if (lpszDirectory[1] == 0)
  {
    return true;
  }
  return lpszDirectory[1] == L'.' && lpszDirectory[2] == 0;
}

bool winDirectoryLister::GetNext(DirectoryEntry2 & direntry2)
{
  WIN32_FIND_DATAW ffdat;
  do
  {
    if (handle == INVALID_HANDLE_VALUE)
    {
      PathName pathPattern(directory);
      if (pattern.empty())
      {
        pathPattern /= "*";
      }
      else
      {
        pathPattern /= pattern.c_str();
      }
      handle = FindFirstFileExW(
        UW_(pathPattern.Get()),
        WindowsVersion::IsWindows7OrGreater() ? FindExInfoBasic : FindExInfoStandard,
        &ffdat,
        (options & (int)Options::DirectoriesOnly) != 0 ? FindExSearchLimitToDirectories : FindExSearchNameMatch,
        nullptr,
        WindowsVersion::IsWindows7OrGreater() ? FIND_FIRST_EX_LARGE_FETCH : 0);
      if (handle == INVALID_HANDLE_VALUE)
      {
        if (::GetLastError() != ERROR_FILE_NOT_FOUND)
        {
          MIKTEX_FATAL_WINDOWS_ERROR_2("FindFirstFileExW", "directory", directory.ToString());
        }
        return false;
      }
    }
    else
    {
      if (!FindNextFileW(handle, &ffdat))
      {
        if (::GetLastError() != ERROR_NO_MORE_FILES)
        {
          MIKTEX_FATAL_WINDOWS_ERROR_2("FindNextFileExW", "directory", directory.ToString());
        }
        return false;
      }
    }
  } while ((((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && IsDotDirectory(ffdat.cFileName))
    || (((options & (int)Options::DirectoriesOnly) != 0) && ((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
    || (((options & (int)Options::FilesOnly) != 0) && ((ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)));
  direntry2.wname = ffdat.cFileName;
  direntry2.name = WU_(ffdat.cFileName);
  direntry2.isDirectory = (ffdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  direntry2.size = ffdat.nFileSizeLow;
  // TODO: large file support
  if (ffdat.nFileSizeHigh != 0)
  {
    MIKTEX_UNEXPECTED();
  }
  return true;
}
