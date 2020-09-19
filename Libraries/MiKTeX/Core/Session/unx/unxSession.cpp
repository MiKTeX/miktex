/* unxSession.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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
      myProgramFile = GetFullyQualifiedPath(invocationName.c_str());
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

#define MIKTEX_MACOS_MIKTEX_LIBRARY_FOLDER "Library/Application Support/MiKTeX"

/*
 * UserConfig:    $HOME/.miktex/texmfs/config
 * UserData:      $HOME/.miktex/texmfs/data
 * UserInstall:   $HOME/.miktex/texmfs/install
 * CommonConfig:  /var/lib/miktex-texmf           (DEB,RPM)
 *             or /var/local/lib/miktex-texmf     (TGZ)
 *             or /opt/miktex/texmfs/config       (self-contained)
 * CommonData:    /var/cache/miktex-texmf         (DEB,RPM,TGZ)
 *             or /opt/miktex/texmfs/data         (self-contained)
 * CommonInstall: /usr/local/share/miktex-texmf   (DEB,RPM,TGZ)
 *             or /opt/miktex/texmfs/install      (self-contained)
 */
VersionedStartupConfig SessionImpl::DefaultConfig(MiKTeXConfiguration config, VersionNumber setupVersion, const PathName& commonPrefixArg, const PathName& userPrefixArg)
{
  VersionedStartupConfig ret;
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
#if defined(MIKTEX_MACOS_BUNDLE)
  home_miktex /= MIKTEX_MACOS_MIKTEX_LIBRARY_FOLDER;
#else
  home_miktex /= ".miktex";
#endif
  PathName home_miktex_texmfs(home_miktex);
  home_miktex_texmfs /= "texmfs";
  ret.userConfigRoot = home_miktex_texmfs;
  ret.userConfigRoot /= "config";
  ret.userDataRoot = home_miktex_texmfs;
  ret.userDataRoot /= "data";
  ret.userInstallRoot = home_miktex_texmfs;
  ret.userInstallRoot /= "install";
#if !defined(MIKTEX_MACOS_BUNDLE)
  PathName prefix = GetMyPrefix(false);
  vector<string> splittedPrefix = PathName::Split(prefix);
  size_t n = splittedPrefix.size();
  MIKTEX_ASSERT(n > 0 && splittedPrefix[0] == "/");
  size_t pos = n;
  if (n > 1 && splittedPrefix[n - 1] == "usr")
  {
    pos = n - 1;
  }
  else if (n > 2 && splittedPrefix[n - 2] == "usr" && splittedPrefix[n - 1] == "local")
  {
    pos = n - 2;
  }
  if (pos < n)
  {
    PathName destdir;
    for (size_t i = 0; i < pos; ++i)
    {
      destdir /= splittedPrefix[i];
    }
    MIKTEX_ASSERT(MIKTEX_SYSTEM_VAR_LIB_DIR[0] == '/');
    ret.commonConfigRoot = destdir / PathName(MIKTEX_SYSTEM_VAR_LIB_DIR + 1) / PathName(MIKTEX_PREFIX "texmf");
    MIKTEX_ASSERT(MIKTEX_SYSTEM_VAR_CACHE_DIR[0] == '/');
    ret.commonDataRoot = destdir / PathName(MIKTEX_SYSTEM_VAR_CACHE_DIR + 1) / PathName(MIKTEX_PREFIX "texmf");
    ret.commonInstallRoot = destdir / PathName("usr/local") / PathName(MIKTEX_INSTALL_DIR);
  }
#endif
  if (ret.commonConfigRoot.Empty())
  {
#if defined(MIKTEX_MACOS_BUNDLE)
    PathName system_miktex_texmfs("/");
    system_miktex_texmfs /= MIKTEX_MACOS_MIKTEX_LIBRARY_FOLDER;
#else
    if (!PathName::Match("*miktex*", prefix.GetData()))
    {
      // TODO: log funny installation prefix
    }
    PathName system_miktex_texmfs(prefix);
#endif
    system_miktex_texmfs /= "texmfs";
    ret.commonConfigRoot = system_miktex_texmfs / PathName("config");
    ret.commonDataRoot = system_miktex_texmfs / PathName("data");
    ret.commonInstallRoot = system_miktex_texmfs / PathName("install");
  }
  return ret;
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
