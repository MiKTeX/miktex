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

class ProcessOutputTrash :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n) override
  {
    return true;
  }
};

void CollectPathNames(vector<PathName> & pathNames, const PathName & dir, const string & pattern)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir, pattern.c_str());
  DirectoryEntry2 entry;
  while (lister->GetNext(entry))
  {
    pathNames.push_back(dir / entry.name);
  }
}

void GetSnapshot(unordered_set<PathName> & pathNames, const PathName & dir, const string & pattern)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir, pattern.c_str());
  vector<PathName> subDirectories;
  DirectoryEntry2 entry;
  while (lister->GetNext(entry))
  {
    if (entry.isDirectory)
    {
      subDirectories.push_back(dir / entry.name);
    }
    else
    {
      pathNames.insert(dir / entry.name);
    }
  }
  for (const PathName & subDir : subDirectories)
  {
    GetSnapshot(pathNames, subDir, pattern);
  }
}

void GetSnapshot(unordered_set<PathName> & pathNames, const PathName & dir)
{
  GetSnapshot(pathNames, dir, "*");
}

PathName PrettyPath(const PathName & path, const PathName & root)
{
  PathName pretty(Utils::GetRelativizedPath(path.GetData(), root.GetData()));
  return pretty.ToUnix();
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
  if (!printOnly)
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
  if (Directory::Exists(destDir))
  {
    MIKTEX_FATAL_ERROR("destination directory alread exists");
  }
  SetupWorkingDirectory();
  Prepare();
  InstallFileSets();
  InstallFiles("tex", standardTeXPatterns, tds.GetTeXDir());
  InstallFiles("doc", standardDocPatterns, tds.GetDocDir());
  InstallFiles("bib", standardBibPatterns, tds.GetBibDir());
  InstallFiles("bst", standardBstPatterns, tds.GetBstDir());
  InstallFiles("csf", standardCsfPatterns, tds.GetCsfDir());
  InstallFiles("ist", standardIstPatterns, tds.GetIstDir());
  InstallFiles("dvips", standardDvipsPatterns, tds.GetDvipsDir());
  InstallFiles("map", standardMapPatterns, tds.GetMapDir());
  InstallFiles("enc", standardEncPatterns, tds.GetEncDir());
  InstallFiles("mf", standardMfPatterns, tds.GetMfDir());
  InstallFiles("tfm", standardTfmPatterns, tds.GetTfmDir());
  InstallFiles("otf", standardOtfPatterns, tds.GetOtfDir());
  InstallFiles("pfb", standardPfbPatterns, tds.GetPfbDir());
  InstallFiles("afm", standardAfmPatterns, tds.GetAfmDir());
  InstallFiles("vf", standardVfPatterns, tds.GetVfDir());
  InstallFiles("mp", standardMpPatterns, tds.GetMetaPostDir());
  InstallFiles("script", standardScriptPatterns, tds.GetScriptDir());
  CleanupWorkingDirectory();
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
  GetSnapshot(initialWorkDirSnapshot, workDir);
}

void Recipe::CleanupWorkingDirectory()
{
  unordered_set<PathName> currentWorkDirSnapshot;
  GetSnapshot(currentWorkDirSnapshot, workDir);
  vector<PathName> toBeDeleted;
  for (const PathName & path : currentWorkDirSnapshot)
  {
    if (initialWorkDirSnapshot.find(path) == initialWorkDirSnapshot.end())
    {
      toBeDeleted.push_back(path);
    }
  }
  for (const PathName & path : toBeDeleted)
  {
    File::Delete(path);
  }
}

void Recipe::Prepare()
{
  vector<string> actions;
  if (recipe->TryGetValue("prepare", "actions[]", actions))
  {
    for (const string & action : actions)
    {
      DoAction(action);
    }
  }
  RunDtxUnpacker();
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
      PrintOnly(StringUtil::FormatString("move <SRCDIR>/%s <SRCDIR>/%s", Q_(PrettyPath(oldName, workDir)), Q_(PrettyPath(newName, workDir))));
      File::Move(oldName, newName);
    }
  }
  else
  {
    MIKTEX_FATAL_ERROR(T_("unknown action"));
  }
}

void Recipe::RunDtxUnpacker()
{
  string engine;
  if (!recipe->TryGetValue("ins", "engine", engine))
  {
    engine = standardInsEngine;
  }
  vector<string> options;
  if (!recipe->TryGetValue("ins", "options[]", options))
  {
    options = standardInsOptions;
  }
  vector<string> patterns;
  if (!recipe->TryGetValue("patterns", "ins[]", patterns))
  {
    patterns = standardInsPatterns;
  }
  vector<PathName> insFiles;
  for (const string & pat : patterns)
  {
    PathName pattern(session->Expand(pat.c_str(), this));
    PathName dir(workDir);
    dir /= pattern;
    dir.RemoveFileSpec();
    pattern.RemoveDirectorySpec();
    CollectPathNames(insFiles, dir, pattern.ToString());
  }
  if (insFiles.empty())
  {
    if (!recipe->TryGetValue("patterns", "dtx[]", patterns))
    {
      patterns = standardDtxPatterns;
    }
    for (const string & pat : patterns)
    {
      PathName pattern(session->Expand(pat.c_str(), this));
      PathName dir(workDir);
      dir /= pattern;
      dir.RemoveFileSpec();
      pattern.RemoveDirectorySpec();
      CollectPathNames(insFiles, dir, pattern.ToString());
    }
  }
  unique_ptr<TemporaryFile> alwaysYes = TemporaryFile::Create();
  StreamWriter writer(alwaysYes->GetPathName());
  for (int i = 0; i < 100; ++i)
  {
    writer.WriteLine("y");
  }
  writer.Close();
  unique_ptr<TemporaryDirectory> outDir = TemporaryDirectory::Create();
  for (const PathName & insFile : insFiles)
  {
    CommandLineBuilder cmd;
    cmd.AppendArgument(engine);
    cmd.AppendOption("-disable-installer");
    cmd.AppendOption("-output-directory=", outDir->GetPathName());
    cmd.AppendOption("-aux-directory=", workDir);
    cmd.AppendArguments(options);
    cmd.AppendArgument(insFile);
    cmd.AppendStdinRedirection(alwaysYes->GetPathName());
    ProcessOutputTrash trash;
    Process::ExecuteSystemCommand(cmd.ToString(), nullptr, &trash, workDir.GetData());
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
    vector<PathName> files;
    CollectPathNames(files, dir, pattern.ToString());
    if (!files.empty() && !madeDestDirectory)
    {
      CreateDirectory(destPath);
      madeDestDirectory = true;
    }
    for (const PathName & file : files)
    {
      PathName toPath(destPath / file.GetFileName());
      if (PrintOnly(StringUtil::FormatString("install <SRCDIR>/%s <DSTDIR>/%s", Q_(PrettyPath(file, workDir)), Q_(PrettyPath(toPath, destDir)))))
      {
        if (Directory::Exists(file))
        {
          Directory::Delete(file, true);
        }
        else
        {
          File::Delete(file);
        }
      }
      else
      {
        File::Move(file, toPath);
      }
    }
  }
}
