/* runperl.cpp: running scripts

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

#include "miktex/Core/CommandLineBuilder.h"
#include "miktex/Core/Paths.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

int SessionImpl::RunScript(const string & scriptEngine, const string & scriptEngineArgument, int argc, const char ** argv)
{
  MIKTEX_ASSERT(argc > 0);

  // find engine
  PathName engine;
  PathName scriptEngineWithExeSuffix = scriptEngine;
#if defined(MIKTEX_WINDOWS)
  scriptEngineWithExeSuffix.SetExtension(MIKTEX_EXE_FILE_SUFFIX, false);
#endif
  if (!Utils::FindProgram(scriptEngineWithExeSuffix.ToString(), engine))
  {
    MIKTEX_FATAL_ERROR_2(T_("The script engine could not be found."), "scriptEngine", scriptEngineWithExeSuffix.ToString());
  }

  // determine script name
  char szName[BufferSizes::MaxPath];
  PathName::Split(argv[0], nullptr, 0, szName, BufferSizes::MaxPath, nullptr, 0);

  // get relative script path
  PathName scriptsIni = GetSpecialPath(SpecialPath::DistRoot);
  scriptsIni /= MIKTEX_PATH_SCRIPTS_INI;
  unique_ptr<Cfg> config(Cfg::Create());
  config->Read(scriptsIni, true);
  string relScriptPath;
  if (!config->TryGetValue(scriptEngine, szName, relScriptPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The script is not registered."), "scriptEngine", scriptEngine, "name", szName);
  }
  config = nullptr;

  // find script
  PathName scriptPath;
  if (!FindFile(relScriptPath.c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The script could not be found."), "scriptEngine", scriptEngine, "name", szName, "path", relScriptPath);
  }

  // build command line
  CommandLineBuilder commandLine;
  if (!scriptEngineArgument.empty())
  {
    commandLine.AppendArgument(scriptEngineArgument);
  }
  commandLine.AppendArgument(scriptPath.ToString());
  if (argc > 1)
  {
    commandLine.AppendArguments(argc - 1, &argv[1]);
  }

  int exitCode;

  Process::Run(engine, commandLine.ToString(), nullptr, &exitCode, nullptr);

  return exitCode;
}

#define PERL "perl"

int SessionImpl::RunPerl(int argc, const char ** argv)
{
  return RunScript(PERL, "", argc, argv);
}

#define PYTHON "python"

int SessionImpl::RunPython(int argc, const char ** argv)
{
  return RunScript(PYTHON, "", argc, argv);
}

#define JAVA "java"

int SessionImpl::RunJava(int argc, const char ** argv)
{
  return RunScript(JAVA, "-jar", argc, argv);
}
