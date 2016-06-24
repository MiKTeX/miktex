/* unxSession.cpp:

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
using namespace std;

PathName SessionImpl::GetMyProgramFile(bool canonicalized)
{
  // we do this once
  if (myProgramFile.Empty())
  {
    string invocationName = initInfo.GetProgramInvocationName();
    if (invocationName.empty())
    {
      MIKTEX_FATAL_ERROR(T_("No invocation name has been set."));
    }
    if (Utils::IsAbsolutePath(invocationName.c_str()))
    {
      myProgramFile = invocationName;
    }
    else if (invocationName.length() > 3 && (invocationName.substr(0, 2) == "./" || invocationName.substr(0, 3) == "../"))
    {
      myProgramFile = GetFullPath(invocationName.c_str());
    }
    else if (!Utils::FindProgram(invocationName, myProgramFile))
    {
      MIKTEX_FATAL_ERROR(T_("The invoked program could not be found in the PATH."));
    }
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

/*
 * UserInstall:   $HOME/.miktex/texmfs/install
 * UserConfig:    $HOME/.miktex/texmfs/config
 * UserData:      $HOME/.miktex/texmfs/data
 * CommonInstall: /opt/miktex/texmfs/install
 * CommonConfig:  /opt/miktex/texmfs/config
 * CommonData:    /opt/miktex/texmfs/data
 */
StartupConfig SessionImpl::DefaultConfig(MiKTeXConfiguration config, const PathName & commonPrefixArg, const PathName & userPrefixArg)
{
  StartupConfig ret;
  if (config == MiKTeXConfiguration::None)
  {
    config = MiKTeXConfiguration::Regular;
  }
  ret.config = config;
  string home;
  if (!Utils::GetEnvironmentString("HOME", home))
  {
    MIKTEX_FATAL_ERROR(T_("Environment variable HOME is not set."));
  }
  PathName home_miktex(home);
  home_miktex /= ".miktex";
  PathName home_miktex_texmfs(home_miktex);
  home_miktex_texmfs /= "texmfs";
  ret.userInstallRoot = home_miktex_texmfs;
  ret.userInstallRoot /= "install";
  ret.userConfigRoot = home_miktex_texmfs;
  ret.userConfigRoot /= "config";
  ret.userDataRoot = home_miktex_texmfs;
  ret.userDataRoot /= "data";
  PathName common_miktex_texmfs(GetMyPrefix());
  common_miktex_texmfs /= "texmfs";
  ret.commonInstallRoot = common_miktex_texmfs;
  ret.commonInstallRoot /= "install";
  ret.commonConfigRoot = common_miktex_texmfs;
  ret.commonConfigRoot /= "config";
  ret.commonDataRoot = common_miktex_texmfs;
  ret.commonDataRoot /= "data";
  return ret;
}

bool SessionImpl::GetPsFontDirs(string & psFontDirs)
{
#warning Unimplemented : SessionImpl::GetPsFontDirs
  return false;
}

bool SessionImpl::GetTTFDirs(string & ttfDirs)
{
#warning Unimplemented : SessionImpl::GetTTFDirs
  return false;
}

bool SessionImpl::GetOTFDirs(string & otfDirs)
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
