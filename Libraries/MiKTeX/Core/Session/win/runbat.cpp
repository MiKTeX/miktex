/* runbat.cpp: running Cmd scripts

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

int SessionImpl::RunBatch(int argc, const char ** argv)
{
  MIKTEX_ASSERT(argc > 0);

  // determine batch name
  char szName[BufferSizes::MaxPath];
  PathName::Split(argv[0], nullptr, 0, szName, BufferSizes::MaxPath, nullptr, 0);

  // get relative script path
  PathName scriptsIni = GetSpecialPath(SpecialPath::DistRoot);
  scriptsIni /= MIKTEX_PATH_SCRIPTS_INI;
  unique_ptr<Cfg> config(Cfg::Create());
  config->Read(scriptsIni, true);
  string relScriptPath;
  if (!config->TryGetValue("bat", szName, relScriptPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The Windows command script is not registered."), "script", szName);
  }
  config = nullptr;

  // find batch file
  PathName scriptPath;
  if (!FindFile(relScriptPath.c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The Windows command script file could not be found."), "name", szName, "path", relScriptPath);
  }

  // build command line
  CommandLineBuilder commandLine;
  commandLine.SetQuotingConvention(QuotingConvention::Bat);
  commandLine.AppendArgument(scriptPath);
  if (argc > 1)
  {
    commandLine.AppendArguments(argc - 1, &argv[1]);
  }

  int exitCode;

  Process::ExecuteSystemCommand(commandLine.ToString(), &exitCode);

  return exitCode;
}
