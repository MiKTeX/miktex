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

void Recipe::Verbose(const string & message)
{
  if (verbose)
  {
    cout << message << endl;
  }
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
  InstallFiles("man1", standardMan1Patterns, tds.GetMan1Dir());
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
  Finalize();
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
  if (File::Exists(source))
  {
    Directory::Create(workDir);
    File::Copy(source, workDir / source.GetFileName());
  }
  else
  {
    Directory::Copy(source, workDir, { DirectoryCopyOption::CopySubDirectories });
  }
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
  WriteFiles();
  vector<string> actions;
  if (recipe->TryGetValue("prepare", "actions[]", actions))
  {
    for (const string & action : actions)
    {
      DoAction(action, workDir);
    }
  }
  RunDtxUnpacker();
}

void Recipe::Finalize()
{
  vector<string> actions;
  if (recipe->TryGetValue("finalize", "actions[]", actions))
  {
    for (const string & action : actions)
    {
      DoAction(action, destDir);
    }
  }
}

void Recipe::WriteFiles()
{
  string file;
  string fileName;
  for (int n = 1; recipe->TryGetValue(file = ("file." + std::to_string(n)), "filename", fileName); ++n)
  {
    vector<string> lines;
    if (!recipe->TryGetValue(file, "lines[]", lines))
    {
      MIKTEX_FATAL_ERROR(T_("missing lines"));
    }
    StreamWriter writer(workDir / fileName);
    for (const string & line : lines)
    {
      writer.WriteLine(line);
    }
    writer.Close();
  }
}

void Recipe::DoAction(const string & action, const PathName & actionDir)
{
  vector<string> argv = Split(session->Expand(action.c_str(), this));
  if (argv.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  string actionName = argv[0];
  if (actionName == "copy")
  {
    if (argv.size() != 3)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    PathName existingName = PathName(actionDir) / argv[1];
    PathName newName = PathName(actionDir) / argv[2];
    if (File::Exists(existingName))
    {
      Verbose("copying '" + argv[1] + "' to '" + argv[2] + "'");
      PrintOnly(StringUtil::FormatString("copy %s %s", Q_(PrettyPath(existingName, actionDir)), Q_(PrettyPath(newName, actionDir))));
      File::Copy(existingName, newName);
    }
  }
  else if (actionName == "move")
  {
    if (argv.size() != 3)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    PathName oldName = PathName(actionDir) / argv[1];
    PathName newName = PathName(actionDir) / argv[2];
    if (File::Exists(oldName))
    {
      Verbose("moving file '" + argv[1] + "' to '" + argv[2] + "'");
      PrintOnly(StringUtil::FormatString("move %s %s", Q_(PrettyPath(oldName, actionDir)), Q_(PrettyPath(newName, actionDir))));
      File::Move(oldName, newName);
    }
    else if (Directory::Exists(oldName))
    {
      Verbose("moving directory '" + argv[1] + "' to '" + argv[2] + "'");
      PrintOnly(StringUtil::FormatString("move %s %s", Q_(PrettyPath(oldName, actionDir)), Q_(PrettyPath(newName, actionDir))));
      Directory::Move(oldName, newName);
    }
  }
  else if (actionName == "remove")
  {
    if (argv.size() != 2)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    PathName name = PathName(actionDir) / argv[1];
    if (File::Exists(name))
    {
      Verbose("removing file '" + argv[1] + "'");
      PrintOnly(StringUtil::FormatString("remove %s", Q_(PrettyPath(name, actionDir))));
      File::Delete(name);
    }
    else if (Directory::Exists(name))
    {
      Verbose("removing directory '" + argv[1] + "'");
      PrintOnly(StringUtil::FormatString("remove %s", Q_(PrettyPath(name, actionDir))));
      Directory::Delete(name, true);
    }
  }
  else if (actionName == "unpack")
  {
    if (argv.size() != 2)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    PathName name = PathName(actionDir) / argv[1];
    if (!File::Exists(name))
    {
      MIKTEX_FATAL_ERROR(T_("cannot run unpack action because the file does not exist"));
    }
    Unpack(name);
  }
  else if (actionName == "echo")
  {
    for (int idx = 1; idx < argv.size(); ++ idx)
    {
      cout << (idx == 1 ? "! " : " ") << argv[idx];
    }
    cout << endl;
  }
  else if (actionName == "abort")
  {
    if (argv.size() != 1)
    {
      MIKTEX_FATAL_ERROR(T_("syntax error (action)"));
    }
    cerr << "aborting" << endl;
    throw 1;
  }
  else
  {
    MIKTEX_FATAL_ERROR(T_("unknown action"));
  }
}

void Recipe::Unpack(const PathName & path)
{
  string fileName = path.GetFileName().ToString();
  size_t extPos = fileName.find('.');
  if (extPos == string::npos)
  {
    MIKTEX_FATAL_ERROR(T_("no archive file type"));
  }
  string command;
  PathName extension = fileName.substr(extPos);
  string relPath = Utils::GetRelativizedPath(path.GetData(), workDir.GetData());
  if (extension == ".zip")
  {
    command = string("unzip") + " " + relPath;
  }
  else if (extension == ".tar.gz")
  {
    command = string("tar") + " -xzf " + relPath;
  }
  else if (extension == ".tar.bz2")
  {
    command = string("tar") + " -xjf " + relPath;
  }
  else if (extension == ".tar.lzma" || extension == ".tar.xz")
  {
    command = string("tar") + " -xJf " + relPath;
  }
  else
  {
    MIKTEX_FATAL_ERROR(T_("unknown archive file type"));
  }
  Verbose("unpacking '" + relPath + "'");
  PrintOnly(command);
  int exitCode = 0;
  if (!Process::ExecuteSystemCommand(command, &exitCode, nullptr, workDir.GetData()) || exitCode != 0)
  {
    MIKTEX_FATAL_ERROR(T_("unpack failure"));
  }
}

void Recipe::RunInsEngine(const string & engine, const vector<string> & options, const PathName & insFile, const PathName & outDir )
{
  unique_ptr<TemporaryFile> alwaysYes = TemporaryFile::Create();
  StreamWriter writer(alwaysYes->GetPathName());
  for (int i = 0; i < 100; ++i)
  {
    writer.WriteLine("y");
  }
  writer.Close();
  CommandLineBuilder cmd;
  cmd.AppendArgument(engine);
  cmd.AppendOption("-disable-installer");
  cmd.AppendOption("-output-directory=", outDir);
  cmd.AppendOption("-aux-directory=", workDir);
  cmd.AppendArguments(options);
  cmd.AppendArgument(insFile);
  cmd.AppendStdinRedirection(alwaysYes->GetPathName());
  ProcessOutputTrash trash;
  Verbose("running .ins engine on '" + insFile.GetFileName().ToString() + "'");
  Process::ExecuteSystemCommand(cmd.ToString(), nullptr, &trash, workDir.GetData());
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
  PathName packageInsFile = workDir;
  packageInsFile /= package + ".ins";
  bool packageInsFileExists = File::Exists(packageInsFile);
  unique_ptr<TemporaryDirectory> outDir = TemporaryDirectory::Create();
  for (const PathName & insFile : insFiles)
  {
    RunInsEngine(engine, options, insFile, outDir->GetPathName());
    if (!packageInsFileExists)
    {
      packageInsFileExists = File::Exists(packageInsFile);
      if (packageInsFileExists)
      {
        Verbose("re-running .ins engine because '" + package + ".ins' has been unpacked");
        RunInsEngine(engine, options, packageInsFile, outDir->GetPathName());
      }
    }
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
    if (!files.empty())
    {
      if (!madeDestDirectory)
      {
        CreateDirectory(destPath);
        madeDestDirectory = true;
      }
      Verbose("installing '" + pat + "': " + std::to_string(files.size()) + " file(s)");
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
