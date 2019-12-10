/* gsinfo.cpp: getting Ghostscript information

   Copyright (C) 1996-2019 Christian Schenk

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

#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

inline bool ScanString(const char* lpszString, const char* lpszFormat, unsigned& u1, unsigned& u2)
{
  int n;
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  n = sscanf_s(lpszString, lpszFormat, &u1, &u2);
#else
  n = sscanf(lpszString, lpszFormat, &u1, &u2);
#endif
  if (n < 0 && n != EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("sscanf");
  }
  return n == 2;
}

PathName SessionImpl::GetGhostscript(unsigned long* versionNumber)
{
  MIKTEX_ASSERT_BUFFER_OR_NIL(versionNumber, sizeof(*versionNumber));

  if (pathGsExe.Empty())
  {
    bool found = false;
#if defined(MIKTEX_WINDOWS)
    found = FindFile(MIKTEX_GS_EXE, FileType::EXE, pathGsExe);
#endif
    if (!found)
    {
      static constexpr const char* gsNames[] = {
#if defined(MIKTEX_WINDOWS)
        "gswin32c",
#endif
        "gs"
      };
      for (const string& name : gsNames)
      {
        found = Utils::FindProgram(name, pathGsExe);
        if (found)
        {
          break;
        }
      }
    }
    if (!found)
    {
      MIKTEX_FATAL_ERROR(T_("Ghostscript could not be not found."));
    }
  }

  if (versionNumber != nullptr && gsVersion.n1 == 0)
  {
    int exitCode;
    ProcessOutput<1024> gsOut;
    if (!Process::Run(pathGsExe, vector<string>{ pathGsExe.GetFileNameWithoutExtension().ToString(), "--version" }, &gsOut, &exitCode, nullptr))
    {
      MIKTEX_UNEXPECTED();
    }
    if (exitCode != 0)
    {
      MIKTEX_FATAL_ERROR_2(T_("Ghostscript is not working correctly."), "gsOut", gsOut.StdoutToString());
    }
    gsVersion = VersionNumber::Parse(gsOut.StdoutToString());
    trace_config->WriteFormattedLine("core", T_("Ghostscript version: %s"), gsVersion.ToString().c_str());
  }

  if (versionNumber != nullptr)
  {
    *versionNumber = static_cast<unsigned long>(gsVersion.GetHighWord());
  }

  return pathGsExe;
}
