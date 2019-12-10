/* init.cpp: session initialization

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

#if defined(MIKTEX_UNIX)
#  include <unistd.h>
#endif

#include <fstream>
#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

// FIXME: must be the first MiKTeX header
#include "core-version.h"

#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Environment>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Core/TemporaryDirectory>

#include "internal.h"

#include "Session/SessionImpl.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

weak_ptr<SessionImpl> SessionImpl::theSession;

void Absolutize(string& paths, const PathName& relativeFrom)
{
  vector<string> result;
  for (const string& path : StringUtil::Split(paths, PathName::PathNameDelimiter))
  {
    if (Utils::IsAbsolutePath(path))
    {
      result.push_back(path);
    }
    else
    {
#if MIKTEX_WINDOWS
      MIKTEX_ASSERT(Utils::IsAbsolutePath(relativeFrom));
      PathName absPath(relativeFrom / path);
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
  paths = StringUtil::Flatten(result, PathName::PathNameDelimiter);
}

void Relativize(string& paths, const PathName& relativeFrom)
{
#if MIKTEX_WINDOWS
  vector<string> result;
  for (const string& path : StringUtil::Split(paths, PathName::PathNameDelimiter))
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
  paths = StringUtil::Flatten(result, PathName::PathNameDelimiter);
#else
  UNIMPLEMENTED();
#endif
}

shared_ptr<Session> Session::Create(const Session::InitInfo& initInfo)
{
  MIKTEX_EXPECT(SessionImpl::theSession.expired());
  shared_ptr<SessionImpl> session = make_shared<SessionImpl>();
  SessionImpl::theSession = session;
  session->Initialize(initInfo);
  return session;
}

Session::~Session() noexcept
{
}

shared_ptr<Session> Session::Get()
{
  return SessionImpl::GetSession();
}

shared_ptr<Session> Session::TryGet()
{
  return SessionImpl::TryGetSession();
}

SessionImpl::SessionImpl() :
  // passing an empty string to the locale constructor is ok; it
  // means: "the user's preferred locale" (cf. "The C++ Programming
  // Language, Appendix D: Locales")
#if !defined(__MINGW32__)
  defaultLocale("")
#else
  // FIXME: work around MingW bug
  defaultLocale()
#endif
{
}

SessionImpl::~SessionImpl()
{
  try
  {
    Uninitialize();
  }
  catch (const MiKTeXException& ex)
  {
    try
    {
#if defined(MIKTEX_WINDOWS)
      ostringstream s;
      s << "error: ~Session(): " << ex;
      OutputDebugStringW(StringUtil::UTF8ToWideChar(s.str()).c_str());
#endif
    }
    catch (const exception&)
    {
    }
  }
  catch (const exception& ex)
  {
    try
    {
#if defined(MIKTEX_WINDOWS)
      ostringstream s;
      s << "error: ~Session(): " << ex.what();
      OutputDebugStringW(StringUtil::UTF8ToWideChar(s.str()).c_str());
#endif
    }
    catch (const exception&)
    {
    }
  }
}

void SessionImpl::Initialize(const Session::InitInfo& initInfo)
{
  adminMode = initInfo.GetOptions()[InitOption::AdminMode];
  if (!adminMode)
  {
    // program name ends with "-admin"?
    PathName programInvocationName = PathName(initInfo.GetProgramInvocationName()).GetFileNameWithoutExtension();
    adminMode = EndsWith(programInvocationName.ToString(), MIKTEX_ADMIN_SUFFIX);
#if defined(MIKTEX_WINDOWS)
    if (!adminMode)
    {
      programInvocationName = GetMyProgramFile(false).TransformForComparison().GetFileNameWithoutExtension();
      adminMode = EndsWith(programInvocationName.ToString(), MIKTEX_ADMIN_SUFFIX);
    }
#endif
  }

#if defined(MIKTEX_WINDOWS)
  if (initInfo.GetOptions()[InitOption::InitializeCOM])
  {
    MyCoInitialize();
  }
#endif

  initialized = true;

  this->initInfo = initInfo;

  theNameOfTheGame = initInfo.GetTheNameOfTheGame();

  RegisterLibraryTraceStreams();

  // enable trace streams
  string traceOptions;
  traceOptions = initInfo.GetTraceFlags();
  if (traceOptions.empty())
  {
    Utils::GetEnvironmentString(MIKTEX_ENV_TRACE, traceOptions);
  }
#if defined(MIKTEX_WINDOWS)
  if (traceOptions.empty() && (!initInfo.GetOptions()[InitOption::NoConfigFiles]))
  {
    if (!winRegistry::TryGetRegistryValue(ConfigurationScope::User, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_TRACE, traceOptions))
    {
      traceOptions = "";
    }
  }
#endif
  if (!traceOptions.empty())
  {
    TraceStream::SetOptions(traceOptions);
  }

  InitializeStartupConfig();

  InitializeRootDirectories(initStartupConfig, false);

  Utils::GetEnvironmentString(MIKTEX_ENV_PACKAGE_LIST_FILE, packageHistoryFile);

  PushAppName(Utils::GetExeName());

  startDirectory.SetToCurrentDirectory();

  string miktexCwd;
  if (Utils::GetEnvironmentString(MIKTEX_ENV_CWD_LIST, miktexCwd))
  {
    for (const string& cwd : StringUtil::Split(miktexCwd, PathName::PathNameDelimiter))
    {
      AddInputDirectory(cwd, true);
    }
  }

  SetEnvironmentVariables();

  trace_core->WriteLine("core", fmt::format(T_("initializing MiKTeX Core version {0}"), MIKTEX_COMPONENT_VERSION_STR));

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
  if (dynamicLoad == TriState::True)
  {
    trace_core->WriteLine("core", T_("dynamic load"));
  }
#endif

  trace_core->WriteLine("core", fmt::format(T_("operating system: {0}"), Q_(Utils::GetOSVersionString())));
  trace_core->WriteLine("core", fmt::format(T_("program file: {0}"), Q_(GetMyProgramFile(true))));
  trace_core->WriteLine("core", fmt::format(T_("current directory: {0}"), Q_(PathName().SetToCurrentDirectory())));
  trace_config->WriteLine("core", fmt::format(T_("admin mode: {0}"), IsAdminMode() ? T_("yes") : T_("no")));
  trace_config->WriteLine("core", fmt::format(T_("shared setup: {0}"), IsSharedSetup() ? T_("yes") : T_("no")));

  trace_config->WriteLine("core", fmt::format(T_("session locale: {0}"), Q_(defaultLocale.name())));

  if (IsAdminMode() && !IsSharedSetup())
  {
    MIKTEX_FATAL_ERROR(T_("Administrator mode startup refused because this is not a shared MiKTeX setup."));
  }

#if 1 // experimental
  if (RunningAsAdministrator() && IsSharedSetup() && !IsAdminMode())
  {
    if (GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_AUTOADMIN).GetTriState() == TriState::True)
    {
      SetAdminMode(true, false);
    }
  }
#endif
}

void SessionImpl::InitializeStartupConfig()
{
  // evaluate init info
  MergeStartupConfig(initStartupConfig, initInfo.GetStartupConfig());

  // read common environment variables
  MergeStartupConfig(initStartupConfig, ReadEnvironment(ConfigurationScope::Common));

  // read user environment variables
  MergeStartupConfig(initStartupConfig, ReadEnvironment(ConfigurationScope::User));

  PathName commonStartupConfigFile;

  bool haveCommonStartupConfigFile = FindStartupConfigFile(ConfigurationScope::Common, commonStartupConfigFile);

  PathName commonPrefix;

  if (haveCommonStartupConfigFile)
  {
    PathName dir(commonStartupConfigFile);
    dir.RemoveFileSpec();
    Utils::GetPathNamePrefix(dir, MIKTEX_PATH_MIKTEX_CONFIG_DIR, commonPrefix);
  }

  PathName userStartupConfigFile;

  bool haveUserStartupConfigFile = FindStartupConfigFile(ConfigurationScope::User, userStartupConfigFile);

  PathName userPrefix;

  if (haveUserStartupConfigFile)
  {
    PathName dir(userStartupConfigFile);
    dir.RemoveFileSpec();
    Utils::GetPathNamePrefix(dir, MIKTEX_PATH_MIKTEX_CONFIG_DIR, userPrefix);
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

#if !NO_REGISTRY
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

  // merge in the default settings
  MergeStartupConfig(initStartupConfig, DefaultConfig(initStartupConfig.config, commonPrefix, userPrefix));
}

StartupConfig SessionImpl::ReadEnvironment(ConfigurationScope scope)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  StartupConfig ret;

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

StartupConfig SessionImpl::ReadStartupConfigFile(ConfigurationScope scope, const PathName& path)
{
  StartupConfig ret;

  unique_ptr<Cfg> cfg(Cfg::Create());

  cfg->Read(path);

  string str;

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
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_COMMON_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_OTHER_COMMON_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.otherCommonRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_COMMON_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonInstallRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_COMMON_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonDataRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_COMMON_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonConfigRoot = str;
    }
  }
  else if (scope == ConfigurationScope::User)
  {
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_USER_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.userRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_OTHER_USER_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.otherUserRoots = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_USER_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.userInstallRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_USER_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.userDataRoot = str;
    }
    if (cfg->TryGetValueAsString("Paths", MIKTEX_REGVAL_USER_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.userConfigRoot = str;
    }
  }

  cfg = nullptr;

  // inherit to child processes
  Utils::SetEnvironmentString(scope == ConfigurationScope::Common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, path.ToString());

  return ret;
}

void SessionImpl::SaveStartupConfig(const MiKTeX::Core::StartupConfig& startupConfig, RegisterRootDirectoriesOptionSet options)
{
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
  if (startupConfig.config != MiKTeXConfiguration::Portable)
  {
    PathName userStartupConfigFile;
    bool haveUserStartupConfigFile = IsAdminMode() ? false : FindStartupConfigFile(ConfigurationScope::User, userStartupConfigFile);
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

void SessionImpl::RecordMaintenance()
{
  time_t now = time(nullptr);
  string nowStr = std::to_string(now);
  if (IsAdminMode())
  {
    SetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, nowStr);
  }
  else
  {
    SetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, nowStr);
  }
}

PathName SessionImpl::GetStartupConfigFile(ConfigurationScope scope, MiKTeXConfiguration config)
{
  StartupConfig defaultConfig = DefaultConfig(config, "", "");
  if (scope == ConfigurationScope::User)
  {
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_STARTUP_FILE, str))
    {
      return str;
    }
#if !NO_REGISTRY
    else if (winRegistry::TryGetRegistryValue(ConfigurationScope::User, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
    {
      return str;
    }
#endif
    else
    {
      return defaultConfig.userConfigRoot / MIKTEX_PATH_STARTUP_CONFIG_FILE;
    }
  }
  else
  {
    string str;
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_STARTUP_FILE, str))
    {
      return str;
    }
#if !NO_REGISTRY
    else if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
    {
      return str;
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
      return prefix / MIKTEX_PATH_STARTUP_CONFIG_FILE;
#else
      // TODO: /etc/miktex/miktexstartup.ini
      return defaultConfig.commonConfigRoot / MIKTEX_PATH_STARTUP_CONFIG_FILE;
#endif
    }
  }
}

void SessionImpl::WriteStartupConfigFile(ConfigurationScope scope, const StartupConfig& startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  StartupConfig defaultConfig = DefaultConfig(startupConfig.config, "", "");

  PathName userStartupConfigFile = GetStartupConfigFile(ConfigurationScope::User, startupConfig.config);  
  PathName commonStartupConfigFile = GetStartupConfigFile(ConfigurationScope::Common, startupConfig.config);
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

  if (scope == ConfigurationScope::Common || allInOne)
  {
    if (!startupConfig.commonRoots.empty() || showAllValues)
    {
      string val = startupConfig.commonRoots;
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_ROOTS, val, T_("common TEXMF root directories"), startupConfig.commonRoots.empty());
    }
    if (!startupConfig.otherCommonRoots.empty() || showAllValues)
    {
      string val = startupConfig.otherCommonRoots;
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_OTHER_COMMON_ROOTS, val, T_("other common TEXMF root directories"), startupConfig.otherCommonRoots.empty());
    }
    if (!startupConfig.commonInstallRoot.Empty() && (startupConfig.commonInstallRoot != defaultConfig.commonInstallRoot || showAllValues))
    {
      string val = startupConfig.commonInstallRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_INSTALL, val, T_("common install root"), startupConfig.commonInstallRoot == defaultConfig.commonInstallRoot);
    }
    if (!startupConfig.commonDataRoot.Empty() && (startupConfig.commonDataRoot != defaultConfig.commonDataRoot || showAllValues))
    {
      string val = startupConfig.commonDataRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_DATA, val, T_("common data root"), startupConfig.commonDataRoot == defaultConfig.commonDataRoot);
    }
    if (!startupConfig.commonConfigRoot.Empty() && (startupConfig.commonConfigRoot != defaultConfig.commonConfigRoot || showAllValues))
    {
      string val = startupConfig.commonConfigRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_CONFIG, val, T_("common config root"), startupConfig.commonConfigRoot == defaultConfig.commonConfigRoot);
    }
  }

  if (scope == ConfigurationScope::User || allInOne)
  {
    if (!startupConfig.userRoots.empty() || showAllValues)
    {
      string val = startupConfig.userRoots;
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_USER_ROOTS, val, T_("user TEXMF root directories"), startupConfig.userRoots.empty());
    }
    if (!startupConfig.otherUserRoots.empty() || showAllValues)
    {
      string val = startupConfig.otherUserRoots;
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_USER_ROOTS, val, T_("other user TEXMF root directories"), startupConfig.otherUserRoots.empty());
    }
    if (!startupConfig.userInstallRoot.Empty() && (startupConfig.userInstallRoot != defaultConfig.userInstallRoot || showAllValues))
    {
      string val = startupConfig.userInstallRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_USER_INSTALL, val, T_("user install root"), startupConfig.userInstallRoot == defaultConfig.userInstallRoot);
    }
    if (!startupConfig.userDataRoot.Empty() && (startupConfig.userDataRoot != defaultConfig.userDataRoot || showAllValues))
    {
      string val = startupConfig.userDataRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_USER_DATA, val, T_("user data root"), startupConfig.userDataRoot == defaultConfig.userDataRoot);
    }
    if (!startupConfig.userConfigRoot.Empty() && (startupConfig.userConfigRoot != defaultConfig.userConfigRoot || showAllValues))
    {
      string val = startupConfig.userConfigRoot.ToString();
      if (!relativeFrom.Empty())
      {
        Relativize(val, relativeFrom);
      };
      cfg->PutValue("Paths", MIKTEX_REGVAL_USER_CONFIG, val, T_("user config root"), startupConfig.userConfigRoot == defaultConfig.userConfigRoot);
    }
  }

  cfg->Write(scope == ConfigurationScope::Common ? commonStartupConfigFile : userStartupConfigFile, T_("MiKTeX startup information"));
}

void SessionImpl::MergeStartupConfig(StartupConfig& startupConfig, const StartupConfig& defaults)
{
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

void SessionImpl::Uninitialize()
{
  if (!initialized)
  {
    return;
  }
  try
  {
    StartFinishScript(10);
    initialized = false;
    trace_core->WriteLine("core", T_("uninitializing core library"));
    CheckOpenFiles();
    WritePackageHistory();
    inputDirectories.clear();
    UnregisterLibraryTraceStreams();
    configurationSettings.clear();
  }
  catch (const exception&)
  {
#if defined(MIKTEX_WINDOWS)
    while (numCoInitialize > 0)
    {
      MyCoUninitialize();
    }
#endif
    throw;
  }
#if defined(MIKTEX_WINDOWS)
  while (numCoInitialize > 0)
  {
    MyCoUninitialize();
  }
#endif
}

void SessionImpl::ScheduleSystemCommand(const std::string& commandLine)
{
  onFinishScript.push_back(commandLine);
}

void SessionImpl::StartFinishScript(int delay)
{
  if (onFinishScript.empty())
  {
    return;
  }
  trace_core->WriteLine("core", fmt::format(T_("finish script: {0} commands to execute"), onFinishScript.size()));
  unique_ptr<TemporaryDirectory> tmpdir = TemporaryDirectory::Create();
  trace_core->WriteLine("core", fmt::format(T_("finish script: tmpdir={0}"), tmpdir->GetPathName()));
  vector<string> pre = {
#if defined(MIKTEX_WINDOWS)
    fmt::format("ping localhost -n {} >nul", delay),
    fmt::format("pushd {}", Q_(tmpdir->GetPathName().ToDos())),
#else
    "#!/bin/sh",
    fmt::format("wait {}", getpid()),
    fmt::format("pushd {}", Q_(tmpdir->GetPathName())),
#endif
  };
  vector<string> post = {
#if defined(MIKTEX_WINDOWS)
    "popd",
#if !MIKTEX_KEEP_FINISH_SCRIPT
    fmt::format("start \"\" /B cmd /C rmdir /S /Q {}", Q_(tmpdir->GetPathName().ToDos())),
#endif
#else
    "popd",
#if !MIKTEX_KEEP_FINISH_SCRIPT
    fmt::format("rm -fr {}", Q_(tmpdir->GetPathName())),
#endif
#endif
  };
  PathName script = tmpdir->GetPathName() / GetMyProgramFile(false).GetFileNameWithoutExtension();
  script += "-finish";
#if defined(MIKTEX_WINDOWS)
  script.SetExtension(".cmd");
#endif
  ofstream writer = File::CreateOutputStream(script);
  for (const auto& cmd : pre)
  {
    writer << cmd << "\n";
  }
  for (const auto& cmd : onFinishScript)
  {
    writer << cmd << "\n";
  }
  for (const auto& cmd : post)
  {
    writer << cmd << "\n";
  }
  writer.close();
  trace_core->WriteLine("core", T_("starting finish script"));
#if defined(MIKTEX_UNIX)
  File::SetAttributes(script, { FileAttribute::Executable });
  Process::Start(script);
#else
  Process::StartSystemCommand(script.ToString());
#endif
  tmpdir->Keep();
}

void SessionImpl::Reset()
{
  vector<string> onFinishScript = move(this->onFinishScript);
  InitInfo initInfo = this->initInfo;
  this->~SessionImpl();
  new (this) SessionImpl();
  Initialize(initInfo);
  this->onFinishScript = move(onFinishScript);
}

void SessionImpl::SetEnvironmentVariables()
{
#if MIKTEX_WINDOWS
  Utils::SetEnvironmentString("TEXSYSTEM", "miktex");

  // Ghostscript
  Utils::SetEnvironmentString("GSC", MIKTEX_GS_EXE);
#endif

  vector<string> gsDirectories;
  PathName gsDir = GetSpecialPath(SpecialPath::CommonInstallRoot) / "ghostscript" / "base";
  if (Directory::Exists(gsDir))
  {
    gsDirectories.push_back(gsDir.ToString());
  }
  if (!IsAdminMode() && GetUserInstallRoot() != GetCommonInstallRoot())
  {
    gsDir = GetSpecialPath(SpecialPath::UserInstallRoot) / "ghostscript" / "base";
    if (Directory::Exists(gsDir))
    {
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  gsDir = GetSpecialPath(SpecialPath::CommonInstallRoot) / "fonts";
  if (Directory::Exists(gsDir))
  {
    gsDirectories.push_back(gsDir.ToString());
  }
  if (!IsAdminMode() && GetUserInstallRoot() != GetCommonInstallRoot())
  {
    gsDir = GetSpecialPath(SpecialPath::UserInstallRoot) / "fonts";
    if (Directory::Exists(gsDir))
    {
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  MIKTEX_ASSERT(!gsDirectories.Empty());

#if defined(MIKTEX_WINDOWS)
  Utils::SetEnvironmentString("MIKTEX_GS_LIB", StringUtil::Flatten(gsDirectories, PathName::PathNameDelimiter));
#else
  string origGsLib;
  if (Utils::GetEnvironmentString("GS_LIB", origGsLib))
  {
    vector<string> origGsLibDirectories = StringUtil::Split(origGsLib, PathName::PathNameDelimiter);
    vector<string> toBeAdded;
    for (const string& d1 : origGsLibDirectories)
    {
      bool duplicate = false;
      for (const string& d2 : gsDirectories)
      {
        if (PathName::Compare(d1, d2) == 0)
        {
          duplicate = true;
          break;
        }
      }
      if (!duplicate)
      {
        toBeAdded.push_back(d1);
      }
    }
    gsDirectories.insert(gsDirectories.end(), toBeAdded.begin(), toBeAdded.end());
  }
  Utils::SetEnvironmentString("GS_LIB", StringUtil::Flatten(gsDirectories, PathName::PathNameDelimiter));
#endif

  PathName path = GetTempDirectory();

  if (!HaveEnvironmentString("TEMPDIR") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TEMPDIR", path.ToString());
  }

  if (!HaveEnvironmentString("TMPDIR") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TMPDIR", path.ToString());
  }

  if (!HaveEnvironmentString("TEMP") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TEMP", path.ToString());
  }

  if (!HaveEnvironmentString("TMP") || IsMiKTeXPortable())
  {
    Utils::SetEnvironmentString("TMP", path.ToString());
  }

  if (!HaveEnvironmentString("HOME"))
  {
    Utils::SetEnvironmentString("HOME", GetHomeDirectory().ToString());
  }

  SetCWDEnv();

  if (!initInfo.GetOptions()[InitOption::NoFixPath])
  {
    string envPath;
    if (!GetEnvironmentString("PATH", envPath))
    {
      envPath = "";
    }
    string newEnvPath;
    bool competition;
    auto p = TryGetBinDirectory(true);
    if (p.first && FixProgramSearchPath(envPath, p.second, false, newEnvPath, competition))
    {
      Utils::SetEnvironmentString("PATH", newEnvPath);
      envPath = newEnvPath;
    }
#if !defined(MIKTEX_MACOS_BUNDLE)
    p = TryGetBinDirectory(false);
    if (p.first && FixProgramSearchPath(envPath, p.second, false, newEnvPath, competition))
    {
      Utils::SetEnvironmentString("PATH", newEnvPath);
      envPath = newEnvPath;
    }
#endif
  }
}

void SessionImpl::SetTheNameOfTheGame(const string& name)
{
  fileTypes.clear();
  theNameOfTheGame = name;
}

