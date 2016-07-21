/* Recipe.h:                                            -*- C++ -*-

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

#pragma once

#include "TDS.h"

class Recipe :
  public MiKTeX::Core::HasNamedValues
{
public:
  Recipe(const std::string & package, const MiKTeX::Core::PathName & source, const MiKTeX::Core::PathName & destDir, bool verbose) :
    package(package),
    source(source),
    destDir(destDir),
    tds(package),
    verbose(verbose)
  {
  }

public:
  void Read(const MiKTeX::Core::PathName & path)
  {
    recipe->Read(path);
  }

public:
  void SetFormat(const std::string & format)
  {
    this->format = format;
    tds.SetFormat(format);
  }

public:
  void SetFoundry(const std::string & foundry)
  {
    this->foundry = foundry;
    tds.SetFoundry(foundry);
  }

public:
  void Execute(bool printOnly);

private:
  void Verbose(const std::string & message);

private:
  bool PrintOnly(const std::string & message);

private:
#if defined(CreateDirectory)
#undef CreateDirectory
#endif
  void CreateDirectory(const MiKTeX::Core::PathName & path);

private:
  void SetupWorkingDirectory();

private:
  void CleanupWorkingDirectory();

private:
  void Prepare();

private:
  void Finalize();

private:
  void RunInsEngine(const std::string & engine, const std::vector<std::string> & options, const MiKTeX::Core::PathName & insFile, const MiKTeX::Core::PathName & outDir );

private:
  void RunDtxUnpacker();

private:
  void DoAction(const std::string & action, const MiKTeX::Core::PathName & actionDir);

private:
  void Unpack(const MiKTeX::Core::PathName & path);

private:
  void WriteFiles();
  
private:
  void InstallFiles(const std::string & patternName, const std::vector<std::string> & defaultPatterns, const MiKTeX::Core::PathName & tdsDir);

private:
  void InstallFileSets();

private:
  void Install(const std::vector<std::string> & patterns, const MiKTeX::Core::PathName & tdsDir);

private:
  bool MIKTEXTHISCALL TryGetValue(const std::string & valueName, std::string & value) override
  {
    if (valueName == "package")
    {
      value = package;
      return true;
    }
    else if (valueName == "format")
    {
      value = format;
      return true;
    }
    else if (valueName == "foundry")
    {
      value = foundry;
      return true;
    }
    return false;
  }

private:
  std::string MIKTEXTHISCALL GetValue(const std::string & valueName) override
  {
    MIKTEX_UNEXPECTED();
  }

private:
  std::unique_ptr<MiKTeX::Core::Cfg> recipe = MiKTeX::Core::Cfg::Create();

private:
  std::string package;

private:
  MiKTeX::Core::PathName source;

private:
  MiKTeX::Core::PathName destDir;

private:
  TDS tds;

private:
  std::string format;

private:
  std::string foundry;

private:
  bool verbose;
  
private:
  bool printOnly;

private:
  std::unique_ptr<MiKTeX::Core::TemporaryDirectory> scratchDir;

private:
  MiKTeX::Core::PathName workDir;

private:
  std::unordered_set<MiKTeX::Core::PathName> initialWorkDirSnapshot;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  const std::string standardFormat = "latex";

private:
  const std::string standardFoundry = "public";

private:
  const std::string standardInsEngine = "latex";

private:
  const std::vector<std::string> standardInsOptions = {
    "-interaction=scrollmode",
    "-halt-on-error"
  };

private:
  const std::vector<std::string> standardInsPatterns = { "*.ins" };

private:
  const std::vector<std::string> standardDtxPatterns = { "*.dtx" };

private:
  const std::vector<std::string> standardTeXPatterns = { "*.cls", "*.sty" };

private:
  const std::vector<std::string> standardDocPatterns = { "*.pdf" };

private:
  const std::vector<std::string> standardBibPatterns; // ignore *.bib

private:
  const std::vector<std::string> standardBstPatterns = { "*.bst" };

private:
  const std::vector<std::string> standardCsfPatterns = { "*.csf" };

private:
  const std::vector<std::string> standardIstPatterns = { "*.ist" };

private:
  const std::vector<std::string> standardDvipsPatterns = { "*.pro" };

private:
  const std::vector<std::string> standardMapPatterns = { "*.map" };

private:
  const std::vector<std::string> standardEncPatterns = { "*.enc" };

private:
  const std::vector<std::string> standardMfPatterns = { "*.mf" };

private:
  const std::vector<std::string> standardTfmPatterns = { "*.tfm" };

private:
  const std::vector<std::string> standardOtfPatterns = { "*.otf" };

private:
  const std::vector<std::string> standardPfbPatterns = { "*.pfb" };

private:
  const std::vector<std::string> standardAfmPatterns = { "*.afm" };

private:
  const std::vector<std::string> standardVfPatterns = { "*.vf" };

private:
  const std::vector<std::string> standardMpPatterns = { "*.mp" };

private:
  const std::vector<std::string> standardMan1Patterns = { "*.1" };
  
private:
  const std::vector<std::string> standardScriptPatterns;
};
