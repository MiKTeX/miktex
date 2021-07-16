/* runperl.cpp: running scripts

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

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

tuple<PathName, vector<string>> SessionImpl::GetScript(const string& scriptEngine, const string& name)
{
  PathName scriptsIni;
  if (!FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
  {
    MIKTEX_FATAL_ERROR_2(T_("The '{filename}' configuration file could not be found."), "filename", MIKTEX_SCRIPTS_INI_FILENAME);
  }
  unique_ptr<Cfg> config(Cfg::Create());
  config->Read(scriptsIni, true);
  string value;
  if (!config->TryGetValueAsString(scriptEngine, name, value))
  {
    MIKTEX_FATAL_ERROR_2(T_("The '{filename}' configuration file contains no record for '{name}'."), "filename", MIKTEX_SCRIPTS_INI_FILENAME, "engine", scriptEngine, "name", name);
  }
  string relScriptPath = Expand(value);
  PathName scriptPath;
  if (!FindFile(relScriptPath, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The program '{name}' could not be found."), "engine", scriptEngine, "name", name, "path", relScriptPath);
  }
  vector<string> args{ scriptEngine };
  vector<string> scriptEngineOptions;
  if (!config->TryGetValueAsStringVector(scriptEngine, name + "." + scriptEngine + "." + "options" + "[]", scriptEngineOptions))
  {
    scriptEngineOptions.clear();
  }
  return make_tuple(scriptPath, scriptEngineOptions);
}

int SessionImpl::RunScript(const string& scriptEngine, const string& scriptEngineArgument, int argc, const char** argv)
{
  MIKTEX_ASSERT(argc > 0);

  PathName name = PathName(argv[0]).GetFileNameWithoutExtension();

  PathName scriptEnginePath;
  if (!Utils::FindProgram(scriptEngine, scriptEnginePath))
  {
    MIKTEX_FATAL_ERROR_5(
      T_("The script engine could not be found."),
      T_("MiKTeX could not find the script engine '{scriptEngine}' which is required to execute '{scriptName}'."),
      T_("Make sure '{scriptEngine}' is installed on your system."),
      "script-engine-not-found",
      "scriptEngine", scriptEngine,
      "scriptName", name.ToString());
  }

  PathName scriptPath;
  vector<string> scriptEngineOptions;
  tie(scriptPath, scriptEngineOptions) = GetScript(scriptEngine, name.ToString());

#if defined(MIKTEX_WINDOWS)
  bool isCmd = scriptEnginePath.HasExtension(".bat") || scriptEnginePath.HasExtension(".cmd");
#endif

  vector<string> args;

#if defined(MIKTEX_WINDOWS)
  args.push_back(isCmd ? scriptEnginePath.ToString() : scriptEngine);
#else
  args.push_back(scriptEngine);
#endif

  if (!scriptEngineOptions.empty())
  {
    args.insert(args.end(), scriptEngineOptions.begin(), scriptEngineOptions.end());
  }

  if (!scriptEngineArgument.empty())
  {
    args.push_back(scriptEngineArgument);
  }

  args.push_back(scriptPath.ToString());

  if (argc > 1)
  {
    args.insert(args.end(), &argv[1], &argv[argc]);
  }

  int exitCode;

#if defined(MIKTEX_WINDOWS)
  if (isCmd)
  {
    CommandLineBuilder commandLine;
    commandLine.SetQuotingConvention(QuotingConvention::Bat);
    commandLine.AppendArguments(args);
    Process::ExecuteSystemCommand(commandLine.ToString(), &exitCode);
    return exitCode;
  }
#endif

  Process::Run(scriptEnginePath, args, nullptr, &exitCode, nullptr);

  return exitCode;
}

int SessionImpl::RunPerl(int argc, const char** argv)
{
  return RunScript("perl", "", argc, argv);
}

int SessionImpl::RunPython(int argc, const char** argv)
{
  return RunScript("python", "", argc, argv);
}

int SessionImpl::RunJava(int argc, const char** argv)
{
  return RunScript("java", "-jar", argc, argv);
}
