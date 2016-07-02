/* Recipe.cpp:                                          -*- C++ -*-

   Copyright (C) 2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

#include "Recipe.h"
#include "TDS.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

bool Recipe::PrintOnly(const string & message)
{
  if (printOnly)
  {
    cout << message << endl;
  }
  return printOnly;
}

void Recipe::CreateDirectory(const PathName & path)
{
  if (!PrintOnly(StringUtil::FormatString("create directory %s", Q_(path))))
  {
    Directory::Create(path);
  }
}

void Recipe::Execute(bool printOnly)
{
  this->printOnly = printOnly;
  if (format.empty())
  {
    if (!recipe->TryGetValue("general", "format", format))
    {
      format = standardFormat;
    }
    SetFormat(format);
  }
  CreateDirectory(destDir);
  SetupWorkingDirectory();
  InstallTeXFiles();
  scratchDir = nullptr;
}

void Recipe::SetupWorkingDirectory()
{
  if (printOnly)
  {
    scratchDir = TemporaryDirectory::Create();
    workDir = scratchDir->GetPathName();
  }
  else
  {
    workDir = destDir / tds.GetSourceDir();
  }
  Directory::Copy(sourceDir, workDir, { DirectoryCopyOption::CopySubDirectories });
}

void Recipe::InstallTeXFiles()
{
  vector<string> patterns;
  if (!recipe->TryGetValue("pkgdir", "texfiles[]", patterns))
  {
    patterns = standardTexFiles;
  }
  Install(patterns, tds.GetTeXDir());
}

void Recipe::Install(const vector<string> & patterns, const PathName & tdsDir)
{
  vector<PathName> files;
  vector<PathName> subDirectories;
  for (const string & pat : patterns)
  {
    PathName pattern(session->Expand(pat.c_str(), this));
    PathName dir(sourceDir);
    dir /= pattern;
    dir.RemoveFileSpec();
    pattern.RemoveDirectorySpec();
    unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir, pattern.GetData());
    DirectoryEntry2 entry;
    while (lister->GetNext(entry))
    {
      if (entry.isDirectory)
      {
        subDirectories.push_back(dir / entry.name);
      }
      else
      {
        files.push_back(dir / entry.name);
      }
    }
    lister = nullptr;
  }
  if (files.size() + subDirectories.size() == 0)
  {
    return;
  }
  PathName destPath(destDir);
  destPath /= tdsDir;
  CreateDirectory(destPath);
  for (const PathName & file : files)
  {
    if (!PrintOnly(StringUtil::FormatString("move file %s %s", Q_(file), destPath / file.GetFileName())))
    {
      File::Move(file, destPath / file.GetFileName());
    }
  }
  for (const PathName & dir : subDirectories)
  {
    if (!PrintOnly(StringUtil::FormatString("move directory %s %s", Q_(dir), destPath / dir.GetFileName())))
    {
      File::Move(dir, destPath / dir.GetFileName());
    }
  }
}
