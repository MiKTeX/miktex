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
  if (foundry.empty())
  {
    if (!recipe->TryGetValue("general", "foundry", foundry))
    {
      foundry = standardFoundry;
    }
    SetFoundry(foundry);
  }
  CreateDirectory(destDir);
  SetupWorkingDirectory();
  Prepare();
  InstallFileSets();
  InstallFiles("tex", standardTeXPatterns, tds.GetTeXDir());
  InstallFiles("doc", standardTeXPatterns, tds.GetTeXDir());
  InstallFiles("bib", standardBibPatterns, tds.GetBibDir());
  InstallFiles("bst", standardBstPatterns, tds.GetBstDir());
  InstallFiles("csf", standardCsfPatterns, tds.GetCsfDir());
  InstallFiles("ist", standardIstPatterns, tds.GetIstDir());
  InstallFiles("dvips", standardDvipsPatterns, tds.GetDvipsDir());
  InstallFiles("map", standardMapPatterns, tds.GetMapDir());
  InstallFiles("enc", standardEncPatterns, tds.GetEncDir());
  InstallFiles("mf", standardMfPatterns, tds.GetMfDir());
  InstallFiles("mp", standardMpPatterns, tds.GetMetaPostDir());
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

void Recipe::Prepare()
{
  vector<string> actions;
  if (!recipe->TryGetValue("prepare", "actions[]", actions))
  {
    return;
  }
  for (const string & action : actions)
  {
    DoAction(action);
  }
}

vector<string> Split(const string & s)
{
  vector<string> argv;
  string current;
  for (const char & ch : s)
  {
    if (ch == ' ' || ch == '\t')
    {
      if (!current.empty())
      {
        argv.push_back(current);
        current = "";
      }
    }
    else
    {
      current += ch;
    }
  }
  if (!current.empty())
  {
    argv.push_back(current);
  }
  return argv;
}

void Recipe::DoAction(const string & action)
{
  vector<string> argv = Split(action);
  if (argv.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  string actionName = argv[0];
  if (actionName == "move")
  {
    if (argv.size() != 3)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    PathName oldName = PathName(workDir) / argv[1];
    PathName newName = PathName(workDir) / argv[2];
    if (File::Exists(oldName))
    {
      File::Move(oldName, newName);
    }
  }
  else
  {
    MIKTEX_FATAL_ERROR(T_("unknown action"));
  }
}

void Recipe::InstallFiles(const string & patternName, const vector<string> & defaultPatterns, const PathName & tdsDir)
{
  vector<string> patterns;
  if (!recipe->TryGetValue("patterns", patternName + "[]", patterns))
  {
    patterns = defaultPatterns;
  }
  Install(patterns, tdsDir);
}

void Recipe::InstallFileSets()
{
  string fileset;
  string tdsdir;
  for (int n = 1; recipe->TryGetValue(fileset = ("fileset." + std::to_string(n)), "tdsdir", tdsdir); ++n)
  {
    vector<string> patterns;
    if (!recipe->TryGetValue(fileset, "patterns[]", patterns))
    {
      MIKTEX_FATAL_ERROR(T_("missing file patterns"));
    }
    Install(patterns, session->Expand(tdsdir.c_str(), this));
  }
}

void Recipe::Install(const vector<string> & patterns, const PathName & tdsDir)
{
  PathName destPath(destDir);
  destPath /= tdsDir;
  bool madeDestDirectory = false;
  for (const string & pat : patterns)
  {
    PathName pattern(session->Expand(pat.c_str(), this));
    PathName dir(workDir);
    dir /= pattern;
    dir.RemoveFileSpec();
    pattern.RemoveDirectorySpec();
    unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir, pattern.GetData());
    DirectoryEntry2 entry;
    vector<PathName> files;
    while (lister->GetNext(entry))
    {
      files.push_back(dir / entry.name);
    }
    lister = nullptr;
    if (!files.empty() && !madeDestDirectory)
    {
      CreateDirectory(destPath);
      madeDestDirectory = true;
    }
    for (const PathName & file : files)
    {
      if (PrintOnly(StringUtil::FormatString("move %s %s", Q_(file), destPath / file.GetFileName())))
      {
        File::Delete(file);
      }
      else
      {
        File::Move(file, destPath / file.GetFileName());
      }
    }
  }
}
