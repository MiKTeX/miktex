/* graphics.cpp: graphics utilities

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

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

bool SessionImpl::FindGraphicsRule(const string & fromExt, const string & toExt, string & rule)
{
  return GetSessionValue("Graphics", fromExt + toExt, rule);
}

bool SessionImpl::ConvertToBitmapFile(const PathName & sourceFileName, PathName & destFileName, IRunProcessCallback * callback)
{
  string ext = sourceFileName.GetExtension();

  if (ext.empty())
  {
    MIKTEX_FATAL_ERROR_2(T_("No file name extension in graphics rule."), "path", ext);
  }

  string rule;

  if (!FindGraphicsRule(ext, ".bmp", rule))
  {
    MIKTEX_FATAL_ERROR_2(T_("No conversion rule found."), "path", sourceFileName.ToString());
  }

  destFileName.SetToTempFile();
#if defined(MIKTEX_WINDOWS)
  Utils::RemoveBlanksFromPathName(destFileName);
#endif

  string commandLine;

  for (const char * lpsz = rule.c_str(); *lpsz != 0; ++lpsz)
  {
    if (*lpsz == '%')
    {
      ++lpsz;
      switch (*lpsz)
      {
      case 'i':
        commandLine += sourceFileName.GetData();
	break;
      case 'o':
        commandLine += destFileName.GetData();
	break;
      }
    }
    else
    {
      commandLine += *lpsz;
    }
  }

  bool done = Process::ExecuteSystemCommand(commandLine, nullptr, callback, nullptr);

  if (!done)
  {
    File::Delete(destFileName, { FileDeleteOption::TryHard });
  }

  return done;
}
