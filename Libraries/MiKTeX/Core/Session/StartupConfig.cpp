/* startupconfig.cpp: startup configuration

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/Environment>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void Absolutize(string& paths, const PathName& relativeFrom)
{
  vector<string> result;
  for (const string& path : StringUtil::Split(paths, PathNameUtil::PathNameDelimiter))
  {
    if (PathNameUtil::IsAbsolutePath(path))
    {
      result.push_back(path);
    }
    else
    {
#if MIKTEX_WINDOWS
      MIKTEX_ASSERT(PathNameUtil::IsAbsolutePath(relativeFrom));
      PathName absPath(relativeFrom / PathName(path));
      PathName absPath2;
      MIKTEX_ASSERT(absPath2.GetCapacity() >= MAX_PATH);
      // FIXME: use wchar_t API
      if (!PathCanonicalizeA(absPath2.GetData(), absPath.GetData()))
      {
        absPath2 = absPath;
      }
      result.push_back(absPath2.ToString());
#else
      UNIMPLEMENTED();
#endif
    }
  }
  paths = StringUtil::Flatten(result, PathNameUtil::PathNameDelimiter);
}

void Relativize(string& paths, const PathName& relativeFrom)
{
#if MIKTEX_WINDOWS
  vector<string> result;
  for (const string& path : StringUtil::Split(paths, PathNameUtil::PathNameDelimiter))
  {
    wchar_t szRelPath[MAX_PATH];
    if (PathRelativePathToW(szRelPath, relativeFrom.ToWideCharString().c_str(), FILE_ATTRIBUTE_DIRECTORY, UW_(path), FILE_ATTRIBUTE_DIRECTORY))
    {
      result.push_back(WU_(szRelPath));
    }
    else
    {
      result.push_back(path);
    }
  }
  paths = StringUtil::Flatten(result, PathNameUtil::PathNameDelimiter);
#else
  UNIMPLEMENTED();
#endif
}

void SessionImpl::InitializeStartupConfig()
{
  bool isSettingUp = initInfo.GetOptions()[InitOption::SettingUp];

  if (isSettingUp)
  {
    initStartupConfig.setupVersion = VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_PATCH_VERSION, 0);
  }

  // evaluate startup config given by caller
  MergeStartupConfig(initStartupConfig, initInfo.GetStartupConfig());

  PathName commonPrefix;
  PathName userPrefix;

  if (!isSettingUp)
  {
    // read common environment variables
    MergeStartupConfig(initStartupConfig, ReadEnvironment(ConfigurationScope::Common));

    // read user environment variables
    MergeStartupConfig(initStartupConfig, ReadEnvironment(ConfigurationScope::User));

    PathName commonStartupConfigFile;

    bool haveCommonStartupConfigFile = FindStartupConfigFile(ConfigurationScope::Common, commonStartupConfigFile);

    if (haveCommonStartupConfigFile)
    {
      PathName dir(commonStartupConfigFile);
      dir.RemoveFileSpec();
      Utils::GetPathNamePrefix(dir, PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR), commonPrefix);
    }

    PathName userStartupConfigFile;

    bool haveUserStartupConfigFile = FindStartupConfigFile(ConfigurationScope::User, userStartupConfigFile);

    if (haveUserStartupConfigFile)
    {
      PathName dir(userStartupConfigFile);
      dir.RemoveFileSpec();
      Utils::GetPathNamePrefix(dir, PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR), userPrefix);
    }

    // read common startup config file
    if (haveCommonStartupConfigFile)
    {
      MergeStartupConfig(initStartupConfig, ReadStartupConfigFile(ConfigurationScope::Common, commonStartupConfigFile));
      if (!IsAdminMode())
      {
        MergeStartupConfig(initStartupConfig, ReadStartupConfigFile(ConfigurationScope::User, commonStartupConfigFile));
      }
    }

    // read user startup config file
    if (haveUserStartupConfigFile)
    {
      MergeStartupConfig(initStartupConfig, ReadStartupConfigFile(ConfigurationScope::User, userStartupConfigFile));
    }

    PathName miKTeXConfig;
    if (FindBinRelative(PathName(MIKTEX_PATH_MIKTEX_INI), miKTeXConfig))
    {
      MergeStartupConfig(initStartupConfig, ReadMiKTeXConfig(miKTeXConfig));
    }

  #if USE_WINDOWS_REGISTRY
    if (initStartupConfig.config != MiKTeXConfiguration::Portable)
    {
      // read the registry, if we don't have a startup config file
      if (!haveCommonStartupConfigFile)
      {
        MergeStartupConfig(initStartupConfig, ReadRegistry(ConfigurationScope::Common));
      }
      if (!haveUserStartupConfigFile)
      {
        MergeStartupConfig(initStartupConfig, ReadRegistry(ConfigurationScope::User));
      }
    }
  #endif
  }

  // merge in the default startup config
  MergeStartupConfig(initStartupConfig, DefaultConfig(initStartupConfig.config, initStartupConfig.setupVersion, commonPrefix, userPrefix));
}

bool SessionImpl::FindBinRelative(const PathName& relPath, PathName& path)
{
  // try the prefix of the internal bin directory
  PathName myloc = GetMyLocation(true);
  RemoveDirectoryDelimiter(myloc.GetData());
  PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
  RemoveDirectoryDelimiter(internalBindir.GetData());
  PathName prefix;
  if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
  {
    path = prefix / relPath;
    if (File::Exists(path))
    {
      return true;
    }
  }
  // try the prefix of the bin directory
  PathName bindir(MIKTEX_PATH_BIN_DIR);
  RemoveDirectoryDelimiter(bindir.GetData());
  if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
  {
    path = prefix / relPath;
    if (File::Exists(path))
    {
      return true;
    }
  }
  return false;
}

bool SessionImpl::FindStartupConfigFile(ConfigurationScope scope, PathName& path)
{
  string str;

  if (Utils::GetEnvironmentString(scope == ConfigurationScope::Common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, str))
  {
    path = str;
    // don't check for existence; it's a fatal error (detected later)
    // if the environment value is incorrect
    return true;
  }

#if USE_WINDOWS_REGISTRY
  if (winRegistry::TryGetValue(scope, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_STARTUP_FILE, str))
  {
    // don't check for existence; it's a fatal error (detected later)
    // if the registry value is incorrect
    path = str;
    return true;
  }
#endif

  StartupConfig defaultStartupConfig = DefaultConfig();

  if (scope == ConfigurationScope::Common)
  {
    if (FindBinRelative(PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE), path))
    {
      return true;
    }
    // try /var/lib/miktex-texmf/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.commonConfigRoot / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
    if (File::Exists(path))
    {
      return true;
    }
#if defined(MIKTEX_UNIX) && !defined(MIKTEX_MACOS_BUNDLE)
    // try /usr/share/miktex-texmf/miktex/config/miktexstartup.ini
    auto prefix = GetMyPrefix(true);
    path = prefix / PathName(MIKTEX_INSTALL_DIR) / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
    if (File::Exists(path))
    {
      return true;
    }
#endif
  }

  if (scope == ConfigurationScope::User)
  {
    // try $HOME/.miktex/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.userConfigRoot / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
    if (File::Exists(path))
    {
      return true;
    }
  }

  return false;
}

InternalStartupConfig SessionImpl::ReadEnvironment(ConfigurationScope scope)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  InternalStartupConfig ret;

  string str;

  if (scope == ConfigurationScope::Common)
  {
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_ROOTS, str))
    {
      ret.commonRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_OTHER_COMMON_ROOTS, str))
    {
      ret.otherCommonRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_INSTALL, str))
    {
      ret.commonInstallRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_DATA, str))
    {
      ret.commonDataRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_CONFIG, str))
    {
      ret.commonConfigRoot = str;
    }
  }
  else if (scope == ConfigurationScope::User)
  {
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_ROOTS, str))
    {
      ret.userRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_OTHER_USER_ROOTS, str))
    {
      ret.otherUserRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_INSTALL, str))
    {
      ret.userInstallRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_DATA, str))
    {
      ret.userDataRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_CONFIG, str))
    {
      ret.userConfigRoot = str;
    }
  }

  return ret;
}

InternalStartupConfig SessionImpl::ReadStartupConfigFile(ConfigurationScope scope, const PathName& path)
{
  InternalStartupConfig ret;

  unique_ptr<Cfg> cfg(Cfg::Create());

  cfg->Read(path);

  string str;

  if (cfg->TryGetValueAsString(MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, str))
  {
    ret.setupVersion = VersionNumber::Parse(str);
  }

  if (cfg->TryGetValueAsString("Auto", "Config", str))
  {
    if (str == "Regular")
    {
      ret.config = MiKTeXConfiguration::Regular;
    }
    else if (str == "Portable")
    {
      ret.config = MiKTeXConfiguration::Portable;
    }
    else if (str == "Direct")
    {
      ret.config = MiKTeXConfiguration::Direct;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }

  PathName relativeFrom(path);
  relativeFrom.RemoveFileSpec();

  if (scope == ConfigurationScope::Common)
  {
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_COMMON_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.otherCommonRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_COMMON_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonInstallRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_COMMON_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonDataRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_COMMON_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonConfigRoot = str;
    }
  }
  else if (scope == ConfigurationScope::User)
  {
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_USER_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.userRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.otherUserRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_USER_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.userInstallRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_USER_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.userDataRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_CONFIG_VALUE_USER_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.userConfigRoot = str;
    }
  }

  cfg = nullptr;

#if 0
  // inherit to child processes
  // TODO: why?
  Utils::SetEnvironmentString(scope == ConfigurationScope::Common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, path.ToString());
#endif

  return ret;
}

InternalStartupConfig SessionImpl::ReadMiKTeXConfig(const PathName& path)
{
  InternalStartupConfig ret;

  unique_ptr<Cfg> cfg(Cfg::Create());

  cfg->Read(path);

  string str;

  if (cfg->TryGetValueAsString(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_SHARED_SETUP, str))
  {
    ret.isSharedSetup = ConfigValue(str).GetTriState();
  }
  
  if (ret.isSharedSetup == TriState::Undetermined && cfg->TryGetValueAsString(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, str))
  {
    ret.isSharedSetup = TriState::True;
  }

  cfg = nullptr;

  return ret;
}

void SessionImpl::SaveStartupConfig(const InternalStartupConfig& startupConfig, RegisterRootDirectoriesOptionSet options)
{
  trace_core->WriteLine("core", TraceLevel::Info, fmt::format(T_("saving startup configuration; setupVersion={0}"), startupConfig.setupVersion));
#if defined(MIKTEX_WINDOWS)
  bool noRegistry = options[RegisterRootDirectoriesOption::NoRegistry];
#else
  bool noRegistry = true;
#endif
  if (IsAdminMode() || startupConfig.config == MiKTeXConfiguration::Portable)
  {
    PathName commonStartupConfigFile;
    bool haveCommonStartupConfigFile = FindStartupConfigFile(ConfigurationScope::Common, commonStartupConfigFile);
    if (haveCommonStartupConfigFile || noRegistry)
    {
      WriteStartupConfigFile(ConfigurationScope::Common, startupConfig);
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      WriteRegistry(ConfigurationScope::Common, startupConfig);
#else
      UNIMPLEMENTED();
#endif
    }
  }
  if (!IsAdminMode() && startupConfig.config != MiKTeXConfiguration::Portable)
  {
    PathName userStartupConfigFile;
    bool haveUserStartupConfigFile = FindStartupConfigFile(ConfigurationScope::User, userStartupConfigFile);
    if (haveUserStartupConfigFile || noRegistry)
    {
      WriteStartupConfigFile(ConfigurationScope::User, startupConfig);
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      WriteRegistry(ConfigurationScope::User, startupConfig);
#else
      UNIMPLEMENTED();
#endif
    }
  }
  RecordMaintenance();
}

PathName SessionImpl::GetStartupConfigFile(ConfigurationScope scope, MiKTeXConfiguration config, VersionNumber version)
{
  StartupConfig defaultConfig = DefaultConfig(config, version, PathName(), PathName());
  if (scope == ConfigurationScope::User)
  {
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_STARTUP_FILE, str))
    {
      return PathName(str);
    }
#if USE_WINDOWS_REGISTRY
    else if (winRegistry::TryGetValue(ConfigurationScope::User, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_STARTUP_FILE, str))
    {
      return PathName(str);
    }
#endif
    else
    {
      return defaultConfig.userConfigRoot / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
    }
  }
  else
  {
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_STARTUP_FILE, str))
    {
      return PathName(str);
    }
#if USE_WINDOWS_REGISTRY
    else if (winRegistry::TryGetValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_STARTUP_FILE, str))
    {
      return PathName(str);
    }
#endif
    else
    {
#if defined(MIKTEX_WINDOWS)
      PathName myloc = GetMyLocation(true);
      RemoveDirectoryDelimiter(myloc.GetData());
      PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
      RemoveDirectoryDelimiter(internalBindir.GetData());
      PathName bindir(MIKTEX_PATH_BIN_DIR);
      RemoveDirectoryDelimiter(bindir.GetData());
      PathName prefix;
      if (!Utils::GetPathNamePrefix(myloc, internalBindir, prefix) && !Utils::GetPathNamePrefix(myloc, bindir, prefix))
      {
        MIKTEX_UNEXPECTED();
      }
      return prefix / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
#else
      // TODO: /etc/miktex/miktexstartup.ini
      return defaultConfig.commonConfigRoot / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
#endif
    }
  }
}

void PutPathValue(Cfg* cfg, const string& valueName, const string& pathValue, const string& defaultValue, const PathName& relativeFrom, bool allowEmpty, const string& documentation)
{
  if ((!pathValue.empty() && pathValue != defaultValue) || allowEmpty)
  {
    string val = pathValue;
    if (!relativeFrom.Empty())
    {
      Relativize(val, relativeFrom);
    };
    cfg->PutValue("Paths", valueName, val, documentation, pathValue.empty() || pathValue == defaultValue);
  }
}

void SessionImpl::WriteStartupConfigFile(ConfigurationScope scope, const InternalStartupConfig& startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  InternalStartupConfig defaultConfig = DefaultConfig(startupConfig.config, startupConfig.setupVersion, PathName(), PathName());

  PathName userStartupConfigFile = GetStartupConfigFile(ConfigurationScope::User, startupConfig.config, startupConfig.setupVersion);
  PathName commonStartupConfigFile = GetStartupConfigFile(ConfigurationScope::Common, startupConfig.config, startupConfig.setupVersion);
  bool allInOne = userStartupConfigFile == commonStartupConfigFile;

  unique_ptr<Cfg> cfg(Cfg::Create());

  const bool showAllValues = false;

  PathName relativeFrom;

  if (startupConfig.config == MiKTeXConfiguration::Portable)
  {
    cfg->PutValue("Auto", "Config", "Portable");
    if (allInOne)
    {
      relativeFrom = commonStartupConfigFile;
      relativeFrom.RemoveFileSpec();
    }
  }

  if (!(startupConfig.setupVersion == VersionNumber()))
  {
    cfg->PutValue(MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, startupConfig.setupVersion.ToString());
  }

  if (scope == ConfigurationScope::Common || allInOne)
  {
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_COMMON_ROOTS, startupConfig.commonRoots, defaultConfig.commonRoots, relativeFrom, allInOne, T_("common root directories"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, startupConfig.otherCommonRoots, defaultConfig.otherCommonRoots, relativeFrom, allInOne, T_("other common root directories"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_COMMON_INSTALL, startupConfig.commonInstallRoot.ToString(), defaultConfig.commonInstallRoot.ToString(), relativeFrom, allInOne, T_("common installation root directoriy"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_COMMON_DATA, startupConfig.commonDataRoot.ToString(), defaultConfig.commonDataRoot.ToString(), relativeFrom, allInOne, T_("common data root directory"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_COMMON_CONFIG, startupConfig.commonConfigRoot.ToString(), defaultConfig.commonConfigRoot.ToString(), relativeFrom, allInOne, T_("common configuration root directory"));
#if 1
    if (!allInOne)
    {
      PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots, defaultConfig.userRoots, relativeFrom, allInOne, T_("user root directories"));
      PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots, defaultConfig.otherUserRoots, relativeFrom, allInOne, T_("other user root directories"));
      PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString(), defaultConfig.userInstallRoot.ToString(), relativeFrom, allInOne, T_("user installation root directoriy"));
      PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString(), defaultConfig.userDataRoot.ToString(), relativeFrom, allInOne, T_("user data root directory"));
      PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString(), defaultConfig.userConfigRoot.ToString(), relativeFrom, allInOne,  T_("user configuration root directory"));
    }
#endif
  }

  if (scope == ConfigurationScope::User || allInOne)
  {
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots, defaultConfig.userRoots, relativeFrom, allInOne, T_("user root directories"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots, defaultConfig.otherUserRoots, relativeFrom, allInOne, T_("other user root directories"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString(), defaultConfig.userInstallRoot.ToString(), relativeFrom, allInOne, T_("user installation root directoriy"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString(), defaultConfig.userDataRoot.ToString(), relativeFrom, allInOne, T_("user data root directory"));
    PutPathValue(cfg.get(), MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString(), defaultConfig.userConfigRoot.ToString(), relativeFrom, allInOne,  T_("user configuration root directory"));
  }

  cfg->Write(scope == ConfigurationScope::Common ? commonStartupConfigFile : userStartupConfigFile, T_("MiKTeX startup information"));
}

void SessionImpl::MergeStartupConfig(InternalStartupConfig& startupConfig, const InternalStartupConfig& defaults)
{
  if (startupConfig.isSharedSetup == TriState::Undetermined)
  {
    startupConfig.isSharedSetup = defaults.isSharedSetup;
  }
  if (startupConfig.setupVersion == VersionNumber())
  {
    startupConfig.setupVersion = defaults.setupVersion;
  }
  if (startupConfig.config == MiKTeXConfiguration::None)
  {
    startupConfig.config = defaults.config;
  }
  if (startupConfig.commonRoots.empty())
  {
    startupConfig.commonRoots = defaults.commonRoots;
  }
  if (startupConfig.userRoots.empty())
  {
    startupConfig.userRoots = defaults.userRoots;
  }
  if (startupConfig.otherCommonRoots.empty())
  {
    startupConfig.otherCommonRoots = defaults.otherCommonRoots;
  }
  if (startupConfig.otherUserRoots.empty())
  {
    startupConfig.otherUserRoots = defaults.otherUserRoots;
  }
  if (startupConfig.commonInstallRoot.Empty())
  {
    startupConfig.commonInstallRoot = defaults.commonInstallRoot;
  }
  if (startupConfig.userInstallRoot.Empty())
  {
    startupConfig.userInstallRoot = defaults.userInstallRoot;
  }
  if (startupConfig.commonDataRoot.Empty())
  {
    startupConfig.commonDataRoot = defaults.commonDataRoot;
  }
  if (startupConfig.userDataRoot.Empty())
  {
    startupConfig.userDataRoot = defaults.userDataRoot;
  }
  if (startupConfig.commonConfigRoot.Empty())
  {
    startupConfig.commonConfigRoot = defaults.commonConfigRoot;
  }
  if (startupConfig.userConfigRoot.Empty())
  {
    startupConfig.userConfigRoot = defaults.userConfigRoot;
  }
}

