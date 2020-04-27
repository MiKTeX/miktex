/* Directory.cpp: directory operations

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

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;

void Directory::Create(const PathName& path)
{
  CreateDirectoryPath(path);
}

void Directory::Delete(const PathName& path, bool recursive)
{
  if (recursive)
  {
    vector<PathName> filesToBeDeleted;
    filesToBeDeleted.reserve(10);

    vector<PathName> directoriesToBeDeleted;
    directoriesToBeDeleted.reserve(10);

    unique_ptr<DirectoryLister> dirLister = DirectoryLister::Open(path);
    DirectoryEntry entry;
    while (dirLister->GetNext(entry))
    {
      if (entry.isDirectory)
      {
        directoriesToBeDeleted.push_back(PathName(path, PathName(entry.name)));
      }
      else
      {
        filesToBeDeleted.push_back(PathName(path, PathName(entry.name)));
      }
    }
    dirLister->Close();

    // remove files
    // TODO: async?
    // TODO: range-based for loop
    for (const PathName& f : filesToBeDeleted)
    {
      File::Delete(f, { FileDeleteOption::TryHard });
    }

    // remove directories recursively
    // TODO: async?
    // TODO: range-based for loop
    for (const PathName& d : directoriesToBeDeleted)
    {
      // RECURSION
      Delete(d, true);
    }
  }

  // remove this directory
  Directory::Delete(path);
}

void Directory::Copy(const PathName& source, const PathName& dest, DirectoryCopyOptionSet options)
{
  vector<PathName> files;
  files.reserve(10);

  vector<PathName> subDirectories;
  subDirectories.reserve(10);

  unique_ptr<DirectoryLister> dirLister = DirectoryLister::Open(source);
  DirectoryEntry entry;
  while (dirLister->GetNext(entry))
  {
    if (entry.isDirectory)
    {
      subDirectories.push_back(PathName(entry.name));
    }
    else
    {
      files.push_back(PathName(entry.name));
    }
  }
  dirLister->Close();

  Directory::Create(dest);

  for (const PathName& file : files)
  {
    PathName sourceFile;
    sourceFile = source;
    sourceFile /= file;
    PathName destFile;
    destFile = dest;
    destFile /= file;
    File::Copy(sourceFile, destFile);
  }

  if (options[DirectoryCopyOption::CopySubDirectories])
  {
    for (const PathName& dir : subDirectories)
    {
      PathName sourceDir;
      sourceDir = source;
      sourceDir /= dir;
      PathName destDir;
      destDir = dest;
      destDir /= dir;
      // RECURSION
      Directory::Copy(sourceDir, destDir, options);
    }
  }
}
