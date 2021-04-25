/* unxDirectoryLister.cpp: directory lister

   Copyright (C) 1996-2021 Christian Schenk

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

#include <dirent.h>
#include <sys/stat.h>

#include <miktex/Core/DirectoryLister>

#include "internal.h"

#include "unxDirectoryLister.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory)
{
  return make_unique<unxDirectoryLister>(directory, nullptr, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory, const char* pattern)
{
  return make_unique<unxDirectoryLister>(directory, pattern, (int)Options::None);
}

unique_ptr<DirectoryLister> DirectoryLister::Open(const PathName& directory, const char* pattern, int options)
{
  return make_unique<unxDirectoryLister>(directory, pattern, options);
}

unxDirectoryLister::unxDirectoryLister(const PathName& directory, const char* pattern, int options) :
  directory(directory),
  pattern(pattern == nullptr ? "" : pattern),
  options(options)
{
}

unxDirectoryLister::~unxDirectoryLister() noexcept
{
  try
  {
    Close();
  }
  catch (const exception&)
  {
  }
}

void unxDirectoryLister::Close()
{
  DIR* dir = this->dir;
  if (dir == nullptr)
  {
    return;
  }
  this->dir = nullptr;
  if (closedir(dir) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("closedir", "dir", directory.ToString());
  }
}

bool unxDirectoryLister::GetNext(DirectoryEntry& direntry)
{
  DirectoryEntry2 direntry2;
  if (!GetNext(direntry2, true))
  {
    return false;
  }
  direntry.name = direntry2.name;
  direntry.isDirectory = direntry2.isDirectory;
  return true;
}

bool unxDirectoryLister::GetNext(DirectoryEntry2& direntry2)
{
  return GetNext(direntry2, false);
}

inline bool IsDotDirectory(const char* entry)
{
  if (entry[0] != '.')
  {
    return false;
  }
  if (entry[1] == 0)
  {
    return true;
  }
  return entry[1] == '.' && entry[2] == 0;
}

bool unxDirectoryLister::GetNext(DirectoryEntry2& direntry2, bool simple)
{
  if (dir == nullptr)
  {
    dir = opendir(directory.GetData());
    if (dir == nullptr)
    {
      MIKTEX_FATAL_CRT_ERROR_2("opendir", "dir", directory.ToString());
    }
  }
  struct dirent* dent;
  bool isDirectory;
  do
  {
    int olderrno = errno;
    dent = readdir(dir);
    if (dent == nullptr)
    {
      if (errno != olderrno)
      {
        MIKTEX_FATAL_CRT_ERROR_2("readdir", "dir", directory.ToString());
      }
      return false;
    }
#if defined(HAVE_STRUCT_DIRENT_D_TYPE)
    isDirectory = dent->d_type == DT_DIR;
#else
    if (options != (int)Options::None)
    {
      UNIMPLEMENTED();
    }
#endif
  } while ((IsDotDirectory(dent->d_name) && ((options & (int)Options::IncludeDotAndDotDot) == 0))
           || (!pattern.empty() && !PathName::Match(pattern.c_str(), dent->d_name))
           || ((options & (int)Options::DirectoriesOnly) != 0 && !isDirectory)
           || ((options & (int)Options::FilesOnly) != 0 && isDirectory));
  direntry2.name = dent->d_name;
  bool mustStat = true;
#if defined(HAVE_STRUCT_DIRENT_D_TYPE)
  if (dent->d_type != DT_UNKNOWN)
  {
    direntry2.isDirectory = (dent->d_type == DT_DIR);
    mustStat = false;
  }
#endif
  if (mustStat || !simple)
  {
    struct stat statbuf;
    PathName path = directory / PathName(dent->d_name);
    if (lstat(path.GetData(), &statbuf) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("lstat", "path", path.ToString());
    }
    direntry2.isDirectory = S_ISDIR(statbuf.st_mode) != 0;
    direntry2.size = statbuf.st_size;
  }
  return true;
}
