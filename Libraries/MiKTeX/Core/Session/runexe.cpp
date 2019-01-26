/* runexe.cpp: running executables

   Copyright (C) 2019 Christian Schenk

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

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

int SessionImpl::RunExe(int argc, const char** argv)
{
  MIKTEX_ASSERT(argc > 0);

  PathName name = PathName(argv[0]).GetFileName();

  PathName executablePath;
  vector<string> executableOptions;
  tie(executablePath, executableOptions) = GetScript("exe", name.ToString());

#if !defined(MIKTEX_WINDOWS)
  if (!File::GetAttributes(executablePath)[FileAttribute::Executable])
  {
    MIKTEX_FATAL_ERROR_2(T_("Cannot execute file."), "path", Q_(executablePath));
  }
#endif

  vector<string> args{ name.ToString() };

  if (argc > 1)
  {
    args.insert(args.end(), &argv[1], &argv[argc]);
  }

  int exitCode;

  Process::Run(executablePath, args, nullptr, &exitCode, nullptr);

  return exitCode;
}
