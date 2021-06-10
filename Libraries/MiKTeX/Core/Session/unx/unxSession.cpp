/* unxSession.cpp:

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

#include <unistd.h>

#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

#include <miktex/Core/File>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

PathName SessionImpl::GetMyProgramFile(bool canonicalized)
{
  // we do this once
  if (myProgramFile.Empty())
  {
#if defined(__APPLE__)
    CharBuffer<char> buf;
    uint32_t bufsize = buf.GetCapacity();
    if (_NSGetExecutablePath(buf.GetData(), &bufsize) < 0)
    {
      buf.Reserve(bufsize);
      if (_NSGetExecutablePath(buf.GetData(), &bufsize) != 0)
      {
        MIKTEX_UNEXPECTED();
      }
    }
    myProgramFile = buf.GetData();
#else
    string invocationName = initInfo.GetProgramInvocationName();
    if (invocationName.empty())
    {
      MIKTEX_FATAL_ERROR(T_("No invocation name has been set."));
    }
    if (PathName(invocationName).IsAbsolute())
    {
      myProgramFile = invocationName;
    }
    else if (invocationName.length() > 3 && (invocationName.substr(0, 2) == "./" || invocationName.substr(0, 3) == "../"))
    {
      myProgramFile = invocationName;
      myProgramFile.Convert({ ConvertPathNameOption::MakeFullyQualified });
    }
    else if (!Utils::FindProgram(invocationName, myProgramFile))
    {
      MIKTEX_FATAL_ERROR_2(T_("The invoked program could not be found in the PATH."), "invocationName", invocationName);
    }
#endif
    myProgramFileCanon = myProgramFile;
    myProgramFileCanon.Canonicalize();
  }
  if (canonicalized)
  {
    return myProgramFileCanon;
  }
  else
  {
    return myProgramFile;
  }
}

bool SessionImpl::GetPsFontDirs(string& psFontDirs)
{
#warning Unimplemented : SessionImpl::GetPsFontDirs
  return false;
}

bool SessionImpl::GetTTFDirs(string& ttfDirs)
{
#warning Unimplemented : SessionImpl::GetTTFDirs
  return false;
}

bool SessionImpl::GetOTFDirs(string& otfDirs)
{
#warning Unimplemented : SessionImpl::GetOTFDirs
  return false;
}

bool SessionImpl::RunningAsAdministrator()
{
  return geteuid() == 0;
}

bool SessionImpl::IsUserAnAdministrator()
{
  return getuid() == 0 || geteuid() == 0;
}
