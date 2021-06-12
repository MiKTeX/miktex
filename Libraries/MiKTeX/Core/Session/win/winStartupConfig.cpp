/* winStartupConfog.cpp: startup configuration (Windows specials)

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

#include <Windows.h>
#include <shlobj.h>

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "win/winRegistry.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

/*
 * UserConfig:    %USERPROFILE%\AppData\Roaming\MiKTeX
 * UserData:      %USERPROFILE%\AppData\Local\MiKTeX
 * UserInstall:   %USERPROFILE%\AppData\Local\Programs\MiKTeX
 * CommonConfig:  C:\ProgramData\MiKTeX
 * CommonData:    C:\ProgramData\MiKTeX
 * CommonInstall: C:\Program Files\MiKTeX
 */
InternalStartupConfig SessionImpl::DefaultConfig(MiKTeXConfiguration config, VersionNumber setupVersion, const PathName& commonPrefixArg, const PathName& userPrefixArg)
{
  InternalStartupConfig ret;
  if (config == MiKTeXConfiguration::None)
  {
    config = MiKTeXConfiguration::Regular;
  }
  ret.config = config;
  ret.setupVersion = setupVersion;
  wchar_t szProgramFiles[MAX_PATH];
  PathName programFiles;
  if (SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, szProgramFiles) == S_OK)
  {
    programFiles = szProgramFiles;
  }
  if (config == MiKTeXConfiguration::Portable)
  {
    PathName commonPrefix(commonPrefixArg);
    PathName userPrefix(userPrefixArg);
    if (commonPrefix.Empty() && !userPrefix.Empty())
    {
      commonPrefix = userPrefix;
    }
    else if (userPrefix.Empty() && !commonPrefix.Empty())
    {
      userPrefix = commonPrefix;
    }
    if (!commonPrefix.Empty())
    {
      PathName portableRoot;
      bool isLegacy = !Utils::GetPathNamePrefix(commonPrefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
      ret.commonInstallRoot = commonPrefix;
      if (!isLegacy)
      {
        ret.commonConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
        ret.commonDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
      }
      else
      {
        ret.commonConfigRoot = commonPrefix;
        ret.commonDataRoot = commonPrefix;
      }
    }
    if (!userPrefix.Empty())
    {
      PathName portableRoot;
      bool isLegacy = !Utils::GetPathNamePrefix(userPrefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
      ret.userInstallRoot = userPrefix;
      if (!isLegacy)
      {
        ret.userConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
        ret.userDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
      }
      else
      {
        ret.userConfigRoot = userPrefix;
        ret.userDataRoot = userPrefix;
      }
    }
    if (userPrefix.Empty() || commonPrefix.Empty())
    {
      PathName myloc(GetMyLocation(false));
      PathName prefix;
      if (Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_INTERNAL_BIN_DIR), prefix)
        || Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_BIN_DIR), prefix)
        || Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_MIKTEX_TEMP_DIR), prefix))
      {
        PathName portableRoot;
        bool isLegacy = !Utils::GetPathNamePrefix(prefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
        if (commonPrefix.Empty())
        {
          ret.commonInstallRoot = prefix;
          if (!isLegacy)
          {
            ret.commonConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
            ret.commonDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
          }
          else
          {
            ret.commonConfigRoot = prefix;
            ret.commonDataRoot = prefix;
          }
        }
        if (userPrefix.Empty())
        {
          ret.userInstallRoot = prefix;
          if (!isLegacy)
          {
            ret.userConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
            ret.userDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
          }
          else
          {
            ret.userConfigRoot = prefix;
            ret.userDataRoot = prefix;
          }
        }
      }
    }
  }
  else
  {
    string product;
    bool isLegacy = setupVersion < VersionNumber("20.6");
    if (config == MiKTeXConfiguration::Direct)
    {
      product = "MiKTeXDirect";
      PathName myloc(GetMyLocation(false));
      PathName prefix;
      if (!Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_BIN_DIR), prefix))
      {
        MIKTEX_UNEXPECTED();
      }
      ret.commonInstallRoot = prefix;
    }
    else
    {
      product = "MiKTeX";
      if (!programFiles.Empty())
      {
        ret.commonInstallRoot = programFiles;
        ret.commonInstallRoot /= product;
        if (isLegacy)
        {
          ret.commonInstallRoot += " " MIKTEX_LEGACY_MAJOR_MINOR_STR;
        }
      }
    }
    wchar_t szPath[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.commonDataRoot = szPath;
      ret.commonDataRoot /= product;
      if (isLegacy)
      {
        ret.commonDataRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    ret.commonConfigRoot = ret.commonDataRoot;
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK
      || SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userDataRoot = szPath;
      ret.userDataRoot /= product;
      if (isLegacy)
      {
        ret.userDataRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    if (SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userConfigRoot = szPath;
      ret.userConfigRoot /= product;
      if (isLegacy)
      {
        ret.userConfigRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    ret.userInstallRoot = ret.userConfigRoot;
  }
  if (!programFiles.Empty())
  {
    PathName myLoc = GetMyLocation(true);
    ret.isSharedSetup = Utils::IsParentDirectoryOf(programFiles, myLoc) ? TriState::True : TriState::False;
  }
  return ret;
}

InternalStartupConfig SessionImpl::ReadRegistry(ConfigurationScope scope)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  InternalStartupConfig ret;

  string str;

  if (scope == ConfigurationScope::Common)
  {
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_SHARED_SETUP, str))
    {
      ret.isSharedSetup = ConfigValue(str).GetTriState();
    }
    if (ret.isSharedSetup == TriState::Undetermined && winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, str))
    {
        ret.isSharedSetup = TriState::True;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, str))
    {
      ret.setupVersion = VersionNumber::Parse(str);
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS, str))
    {
      ret.commonRoots = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, str))
    {
      ret.otherCommonRoots = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL, str))
    {
      ret.commonInstallRoot = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA, str))
    {
      ret.commonDataRoot = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG, str))
    {
      ret.commonConfigRoot = str;
    }
  }
  else if (scope == ConfigurationScope::User)
  {
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, str))
    {
      ret.setupVersion = VersionNumber::Parse(str);
    }
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, str))
    {
      ret.userRoots = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, str))
    {
      ret.otherUserRoots = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, str))
    {
      ret.userInstallRoot = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, str))
    {
      ret.userDataRoot = str;
    }
    if (winRegistry::TryGetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, str))
    {
      ret.userConfigRoot = str;
    }
  }

  return ret;
}

void SessionImpl::WriteRegistry(ConfigurationScope scope, const InternalStartupConfig& startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  InternalStartupConfig defaultConfig = DefaultConfig(startupConfig.config, startupConfig.setupVersion, PathName(), PathName());

  // remove registry values
  if (scope == ConfigurationScope::Common)
  {
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS);
#if 1
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG);
    winRegistry::TryDeleteValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS);
#endif
  }
  else if (scope == ConfigurationScope::User)
  {
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION);
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS);
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL);
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA);
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG);
    winRegistry::TryDeleteValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS);
  }

  if (scope == ConfigurationScope::Common)
  {
    if (!(startupConfig.setupVersion == VersionNumber()))
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, startupConfig.setupVersion.ToString());
    }
    if (!startupConfig.commonRoots.empty() && startupConfig.commonRoots != defaultConfig.commonRoots)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS, startupConfig.commonRoots);
    }
    if (!startupConfig.otherCommonRoots.empty() && startupConfig.otherCommonRoots != defaultConfig.otherCommonRoots)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, startupConfig.otherCommonRoots);
    }
    if (!startupConfig.commonInstallRoot.Empty() && startupConfig.commonInstallRoot != defaultConfig.commonInstallRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL, startupConfig.commonInstallRoot.ToString());
    }
    if (!startupConfig.commonDataRoot.Empty() && startupConfig.commonDataRoot != defaultConfig.commonDataRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA, startupConfig.commonDataRoot.ToString());
    }
    if (!startupConfig.commonConfigRoot.Empty() && startupConfig.commonConfigRoot != defaultConfig.commonConfigRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG, startupConfig.commonConfigRoot.ToString());
    }
#if 1
    if (!startupConfig.userRoots.empty() && startupConfig.userRoots != defaultConfig.userRoots)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots);
    }
    if (!startupConfig.otherUserRoots.empty() && startupConfig.otherUserRoots != defaultConfig.otherUserRoots)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots);
    }
    if (!startupConfig.userInstallRoot.Empty() && startupConfig.userInstallRoot != defaultConfig.userInstallRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString());
    }
    if (!startupConfig.userDataRoot.Empty() && startupConfig.userDataRoot != defaultConfig.userDataRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString());
    }
    if (!startupConfig.userConfigRoot.Empty() && startupConfig.userConfigRoot != defaultConfig.userConfigRoot)
    {
      winRegistry::SetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString());
    }
#endif
  }
  else if (scope == ConfigurationScope::User)
  {
    if (!(startupConfig.setupVersion == VersionNumber()))
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, startupConfig.setupVersion.ToString());
    }
    if (!startupConfig.userRoots.empty() && startupConfig.userRoots != defaultConfig.userRoots)
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots);
    }
    if (!startupConfig.otherUserRoots.empty() && startupConfig.otherUserRoots != defaultConfig.otherUserRoots)
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots);
    }
    if (!startupConfig.userInstallRoot.Empty() && startupConfig.userInstallRoot != defaultConfig.userInstallRoot)
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString());
    }
    if (!startupConfig.userDataRoot.Empty() && startupConfig.userDataRoot != defaultConfig.userDataRoot)
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString());
    }
    if (!startupConfig.userConfigRoot.Empty() && startupConfig.userConfigRoot != defaultConfig.userConfigRoot)
    {
      winRegistry::SetValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString());
    }
  }
}
