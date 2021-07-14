/* init.cpp: session initialization

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

#if defined(MIKTEX_UNIX)
#  include <unistd.h>
extern char** environ;
#endif

#if defined(MIKTEX_MACOS_BUNDLE)
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron ())
#endif

#include <fstream>
#include <iostream>

#include <fmt/format.h>
#include <fmt/ostream.h>

// FIXME: must be the first MiKTeX header
#include "core-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/AutoResource>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Environment>
#include <miktex/Core/Paths>
#include <miktex/Core/TemporaryDirectory>

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

weak_ptr<SessionImpl> SessionImpl::theSession;

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


shared_ptr<Session> Session::TryGet()
{
  return SessionImpl::TryGetSession();
}

SessionImpl::SessionImpl()
{
}

SessionImpl::~SessionImpl()
{
  try
  {
    Close();
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

  initialized = true;

  fsWatcher = FileSystemWatcher::Create();
  fsWatcher->Start();

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
  if (traceOptions.empty() && (!initInfo.GetOptions()[InitOption::SettingUp]))
  {
    if (!winRegistry::TryGetValue(ConfigurationScope::User, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_TRACE, traceOptions))
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
    for (const string& cwd : StringUtil::Split(miktexCwd, PathNameUtil::PathNameDelimiter))
    {
      AddInputDirectory(PathName(cwd), true);
    }
  }

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

void SessionImpl::RecordMaintenance()
{
  time_t now = time(nullptr);
  string nowStr = std::to_string(now);
  if (IsAdminMode())
  {
    SetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue(nowStr));
  }
  else
  {
    SetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue(nowStr));
  }
}

void SessionImpl::Close()
{
  if (!initialized)
  {
    return;
  }
  StartFinishScript(10);
  initialized = false;
  trace_core->WriteLine("core", T_("uninitializing core library"));
  if (fsWatcher != nullptr)
  {
    fsWatcher->Stop();
    fsWatcher = nullptr;
  }
  CheckOpenFiles();
  WritePackageHistory();
  inputDirectories.clear();
  UnregisterLibraryTraceStreams();
  configurationSettings.clear();
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
  trace_core->WriteLine("core", TraceLevel::Info, T_("starting finish script"));
#if defined(MIKTEX_UNIX)
  File::SetAttributes(script, { FileAttribute::Executable });
  Process::Start(script);
#else
  Process::StartSystemCommand(script.ToString());
#endif
  tmpdir->Keep();
}

SetupConfig SessionImpl::GetSetupConfig()
{
  SetupConfig ret;
  ret.setupVersion = initStartupConfig.setupVersion;
  ret.isNew = false;
  PathName configDir = GetSpecialPath(IsAdminMode() || IsSharedSetup() ? SpecialPath::CommonInstallRoot : SpecialPath::UserInstallRoot);
  configDir /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  if (Directory::Exists(configDir))
  {
    //                                                012345678901234567890123456789
    auto lister = DirectoryLister::Open(configDir, R"(setup-????-??-??-??-??.log)", (int)DirectoryLister::Options::FilesOnly);
    DirectoryEntry dirEntry;
    if (lister->GetNext(dirEntry))
    {
      struct tm setupDate = tm();
      setupDate.tm_year = std::stoi(dirEntry.name.substr(6, 4)) - 1900;
      setupDate.tm_mon = std::stoi(dirEntry.name.substr(11, 2)) - 1;
      setupDate.tm_mday = std::stoi(dirEntry.name.substr(14, 2));
      setupDate.tm_hour = std::stoi(dirEntry.name.substr(17, 2));
      setupDate.tm_min = std::stoi(dirEntry.name.substr(20, 2));
      setupDate.tm_isdst = -1;
      ret.setupDate = mktime(&setupDate);
      if (ret.setupDate == static_cast<time_t>(-1))
      {
        // TODO
      }
      return ret;
    }
    for (const auto& name : { "miktexstartup.ini" })
    {
      PathName file = configDir / PathName(name);
      if (File::Exists(file))
      {
        ret.setupDate = File::GetLastWriteTime(file);
        return ret;
      }
    }
  }
  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetString()));
  ret.isNew = lastAdminMaintenance == 0 && lastUserMaintenance == 0 && !IsMiKTeXPortable();
  return ret;
}

void SessionImpl::Reset()
{
  vector<string> onFinishScript = move(this->onFinishScript);
  InitInfo initInfo = this->initInfo;
  this->Close();
  this->~SessionImpl();
  new (this) SessionImpl();
  Initialize(initInfo);
  this->onFinishScript = move(onFinishScript);
}

unordered_map<string, string> SessionImpl::CreateChildEnvironment(bool changeDirectory)
{
  unordered_map<string, string> envMap;

#if defined(MIKTEX_WINDOWS)
  auto environmentStrings = GetEnvironmentStringsW();
  MIKTEX_AUTO(FreeEnvironmentStringsW(environmentStrings));
  for (const wchar_t* env = environmentStrings; *env != 0; ++env)
  {
    wstring name;
    if (*env == L'=')
    {
      name += *env++;
    }
    for (; *env != L'=' && *env != 0; ++env)
    {
      name += *env;
    }
    if (*env == L'=')
    {
      ++env;
    }
    wstring value;
    for (; *env != 0; ++env)
    {
      value += *env;
    }
    envMap[WU_(name)] = WU_(value);
  }
#else
  for (char** env = environ; *env != nullptr; ++env)
  {
    string name;
    auto s = *env;
    for (; *s != '=' && *s != 0; ++s)
    {
      name += *s;
    }
    if (*s == '=')
    {
      ++s;
    }
    string value;
    for (; *s != 0; ++s)
    {
      value += *s;
    }
    envMap[name] = value;
  }
#endif

  envMap["TEXSYSTEM"] = "miktex";

#if defined(MIKTEX_WINDOWS)
  envMap["GSC"] = MIKTEX_GS_EXE;
#endif

  vector<string> gsDirectories;
  if (IsSharedSetup())
  {
    PathName gsDir = GetSpecialPath(SpecialPath::CommonInstallRoot) / PathName("ghostscript") / PathName("base");
    if (Directory::Exists(gsDir))
    {
#if defined(MIKTEX_WINDOWS)
      gsDir.ConvertToUnix();
#endif
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  if (!IsAdminMode() && (!IsSharedSetup() || GetUserInstallRoot() != GetCommonInstallRoot()))
  {
    PathName gsDir = GetSpecialPath(SpecialPath::UserInstallRoot) / PathName("ghostscript") / PathName("base");
    if (Directory::Exists(gsDir))
    {
#if defined(MIKTEX_WINDOWS)
      gsDir.ConvertToUnix();
#endif
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  if (IsSharedSetup())
  {
    PathName gsDir = GetSpecialPath(SpecialPath::CommonInstallRoot) / PathName("fonts");
    if (Directory::Exists(gsDir))
    {
#if defined(MIKTEX_WINDOWS)
      gsDir.ConvertToUnix();
#endif
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  if (!IsAdminMode() && (!IsSharedSetup() || GetUserInstallRoot() != GetCommonInstallRoot()))
  {
    PathName gsDir = GetSpecialPath(SpecialPath::UserInstallRoot) / PathName("fonts");
    if (Directory::Exists(gsDir))
    {
#if defined(MIKTEX_WINDOWS)
      gsDir.ConvertToUnix();
#endif
      gsDirectories.push_back(gsDir.ToString());
    }
  }
  MIKTEX_ASSERT(!gsDirectories.empty());

#if defined(MIKTEX_WINDOWS)
  envMap["MIKTEX_GS_LIB"] = StringUtil::Flatten(gsDirectories, PathNameUtil::PathNameDelimiter);
#endif
  auto it = envMap.find("GS_LIB");
  if (it != envMap.end())
  {
    vector<string> origGsLibDirectories = StringUtil::Split(it->second, PathNameUtil::PathNameDelimiter);
    for (const string& d1 : origGsLibDirectories)
    {
      auto it = find_if(gsDirectories.begin(), gsDirectories.end(), [d1](const string& d2) { return PathName::Compare(d1, d2) == 0; });
      if (it == gsDirectories.end())
      {
        gsDirectories.push_back(d1);
      }
    }
  }
  envMap["GS_LIB"] = StringUtil::Flatten(gsDirectories, PathNameUtil::PathNameDelimiter);

  PathName tempDirectory = GetTempDirectory();
  for (const string& envName : vector<string>{ "TEMP", "TEMPDIR", "TMP", "TMPDIR" })
  {
    auto it = envMap.find(envName);
    if (it == envMap.end() || !PathName(it->second).IsAbsolute() || IsMiKTeXPortable())
    {
      envMap[envName] = tempDirectory.ToString();
    }
  }

  for (const string& envName : vector<string>{ "HOME" })
  {
    auto it = envMap.find(envName);
    if (it == envMap.end() || !PathName(it->second).IsAbsolute())
    {
      envMap[envName] = GetHomeDirectory().ToString();
    }
  }

  vector<string> cwdList;
  for (const PathName& dir : inputDirectories)
  {
    cwdList.push_back(dir.ToString());
  }
  if (changeDirectory)
  {
    cwdList.push_back(PathName().SetToCurrentDirectory().ToString());
  }
  envMap[MIKTEX_ENV_CWD_LIST] = StringUtil::Flatten(cwdList, PathNameUtil::PathNameDelimiter);

  if (!initInfo.GetOptions()[InitOption::NoFixPath])
  {
    string envPath;
    auto it = envMap.find("PATH");
    if (it != envMap.end())
    {
      envPath = it->second;
    }
    string newEnvPath;
    bool competition;
    auto p = TryGetBinDirectory(true);
    if (p.first && FixProgramSearchPath(envPath, p.second, false, newEnvPath, competition))
    {
      envMap["PATH"] = newEnvPath;
      envPath = newEnvPath;
    }
#if !defined(MIKTEX_MACOS_BUNDLE)
    p = TryGetBinDirectory(false);
    if (p.first && FixProgramSearchPath(envPath, p.second, false, newEnvPath, competition))
    {
      envMap["PATH"] = newEnvPath;
      envPath = newEnvPath;
    }
#endif
    PathName linkTargetDirectory = GetSpecialPath(SpecialPath::LinkTargetDirectory);
    if (Directory::Exists(linkTargetDirectory) && FixProgramSearchPath(envPath, linkTargetDirectory, false, newEnvPath, competition))
    {
      envMap["PATH"] = newEnvPath;
      envPath = newEnvPath;
    }
  }

  return envMap;
}

void SessionImpl::SetTheNameOfTheGame(const string& name)
{
  fileTypes.clear();
  theNameOfTheGame = name;
}

