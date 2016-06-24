/* texmfroot.cpp: managing TEXMF root directories

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

#include "miktex/Core/CSVList.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#if defined(MIKTEX_WINDOWS)
#  include "miktex/Core/win/HResult.h"
#  include "miktex/Core/win/winAutoResource.h"
#endif

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

// index of the hidden MPM root
#define MPM_ROOT static_cast<unsigned>(GetNumberOfTEXMFRoots())

namespace {
  mutex fndbMutex;
}

static string ExpandEnvironmentVariables(const char * lpszToBeExpanded)
{
  const char * lpsz = lpszToBeExpanded;
  string valueName;
  string expansion;
  expansion.reserve(strlen(lpsz));
  for (; *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '<')
    {
      const char * lpszBegin = lpsz;
      const char endChar = '>';
      valueName = "";
      for (lpsz += 1; *lpsz != 0 && *lpsz != endChar; ++lpsz)
      {
	valueName += *lpsz;
      }
      if (*lpsz != endChar)
      {
	MIKTEX_UNEXPECTED();
      }
      if (valueName.empty())
      {
	MIKTEX_UNEXPECTED();
      }
      string value;
      if (!Utils::GetEnvironmentString(valueName.c_str(), value))
      {
	MIKTEX_FATAL_ERROR_2(T_("Environment variable not defined."), "name", valueName);
      }
      expansion += value;
    }
    else
    {
      expansion += lpsz[0];
    }
  }
  return expansion;
}

unsigned SessionImpl::RegisterRootDirectory(const PathName & root, bool common)
{
  unsigned idx;
  for (idx = 0; idx < rootDirectories.size(); ++idx)
  {
    if (root == rootDirectories[idx].get_UnexpandedPath())
    {
      // already registered
      if (common && !rootDirectories[idx].IsCommon())
      {
	trace_config->WriteFormattedLine("core", T_("now a common TEXMF root: %s"), root.Get());
	rootDirectories[idx].set_Common(common);
      }
      return idx;
    }
  }
  trace_config->WriteFormattedLine("core", T_("registering %s TEXMF root: %s"), common ? "common" : "user", root.Get());
  RootDirectory rootDirectory(root, ExpandEnvironmentVariables(root.Get()));
  rootDirectory.set_Common(common);
  rootDirectories.reserve(10);
  rootDirectories.push_back(rootDirectory);
  return idx;
}

MIKTEXSTATICFUNC(void) MergeStartupConfig(StartupConfig & startupConfig, const StartupConfig & defaults)
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

void SessionImpl::DoStartupConfig()
{
  // evaluate init info
  MergeStartupConfig(startupConfig, initInfo.GetStartupConfig());

  // read common environment variables
  MergeStartupConfig(startupConfig, ReadEnvironment(true));

  // read user environment variables
  MergeStartupConfig(startupConfig, ReadEnvironment(false));

  PathName commonStartupConfigFile;

  bool haveCommonStartupConfigFile = FindStartupConfigFile(true, commonStartupConfigFile);

  PathName commonPrefix;

  if (haveCommonStartupConfigFile)
  {
    PathName dir(commonStartupConfigFile);
    dir.RemoveFileSpec();
    Utils::GetPathNamePrefix(dir, MIKTEX_PATH_MIKTEX_CONFIG_DIR, commonPrefix);
  }

  PathName userStartupConfigFile;

  bool haveUserStartupConfigFile = FindStartupConfigFile(false, userStartupConfigFile);

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
    MergeStartupConfig(startupConfig, ReadStartupConfigFile(true, commonStartupConfigFile));
  }

  // read user startup config file
  if (haveUserStartupConfigFile)
  {
    MergeStartupConfig(startupConfig, ReadStartupConfigFile(false, userStartupConfigFile));
  }

#if ! NO_REGISTRY
  if (startupConfig.config != MiKTeXConfiguration::Portable)
  {
    // read the registry, if we don't have a startup config file
    if (!haveCommonStartupConfigFile)
    {
      MergeStartupConfig(startupConfig, ReadRegistry(true));
    }
    if (!haveUserStartupConfigFile)
    {
      MergeStartupConfig(startupConfig, ReadRegistry(false));
    }
  }
#endif

  // merge in the default settings
  MergeStartupConfig(startupConfig, DefaultConfig(startupConfig.config, commonPrefix, userPrefix));
}

void SessionImpl::InitializeRootDirectories()
{
  InitializeRootDirectories(startupConfig);
}

void SessionImpl::InitializeRootDirectories(const StartupConfig & startupConfig)
{
  rootDirectories.clear();

  commonInstallRootIndex = INVALID_ROOT_INDEX;
  userInstallRootIndex = INVALID_ROOT_INDEX;
  commonDataRootIndex = INVALID_ROOT_INDEX;
  userDataRootIndex = INVALID_ROOT_INDEX;
  commonConfigRootIndex = INVALID_ROOT_INDEX;
  userConfigRootIndex = INVALID_ROOT_INDEX;

  // UserConfig
  if (!startupConfig.userConfigRoot.Empty())
  {
    userConfigRootIndex = RegisterRootDirectory(startupConfig.userConfigRoot, false);
  }

  // UserData
  if (!startupConfig.userDataRoot.Empty())
  {
    userDataRootIndex = RegisterRootDirectory(startupConfig.userDataRoot, false);
  }

  // UserRoots
  for (CSVList root(startupConfig.userRoots, PATH_DELIMITER); root.GetCurrent() != nullptr; ++root)
  {
    if (*root.GetCurrent() != 0)
    {
      RegisterRootDirectory(root.GetCurrent(), false);
    }
  }

  // UserInstall
  if (!startupConfig.userInstallRoot.Empty())
  {
    userInstallRootIndex = RegisterRootDirectory(startupConfig.userInstallRoot, false);
  }

  // CommonConfig
  if (!startupConfig.commonConfigRoot.Empty())
  {
    commonConfigRootIndex = RegisterRootDirectory(startupConfig.commonConfigRoot, true);
  }

  // CommonData
  if (!startupConfig.commonDataRoot.Empty())
  {
    commonDataRootIndex = RegisterRootDirectory(startupConfig.commonDataRoot, true);
  }

  // CommonRoots
  for (CSVList root(startupConfig.commonRoots, PATH_DELIMITER); root.GetCurrent() != nullptr; ++root)
  {
    if (*root.GetCurrent() != 0)
    {
      RegisterRootDirectory(root.GetCurrent(), true);
    }
  }

  // CommonInstall
  if (!startupConfig.commonInstallRoot.Empty())
  {
    commonInstallRootIndex = RegisterRootDirectory(startupConfig.commonInstallRoot, true);
  }

  if (rootDirectories.size() == 0)
  {
    MIKTEX_UNEXPECTED();
  }

  if (commonDataRootIndex == INVALID_ROOT_INDEX)
  {
    commonDataRootIndex = 0;
  }

  if (userDataRootIndex == INVALID_ROOT_INDEX)
  {
    userDataRootIndex = 0;
  }

  if (commonConfigRootIndex == INVALID_ROOT_INDEX)
  {
    commonConfigRootIndex = commonDataRootIndex;
  }

  if (userConfigRootIndex == INVALID_ROOT_INDEX)
  {
    userConfigRootIndex = userDataRootIndex;
  }

  if (commonInstallRootIndex == INVALID_ROOT_INDEX)
  {
    commonInstallRootIndex = commonConfigRootIndex;
  }

  if (userInstallRootIndex == INVALID_ROOT_INDEX)
  {
    userInstallRootIndex = userConfigRootIndex;
  }

  RegisterRootDirectory(MPM_ROOT_PATH, IsAdminMode());

  trace_config->WriteFormattedLine("core", "UserData: %s", GetRootDirectory(userDataRootIndex).Get());

  trace_config->WriteFormattedLine("core", "UserConfig: %s", GetRootDirectory(userConfigRootIndex).Get());

  trace_config->WriteFormattedLine("core", "UserInstall: %s", GetRootDirectory(userInstallRootIndex).Get());

  trace_config->WriteFormattedLine("core", "CommonData: %s", GetRootDirectory(commonDataRootIndex).Get());

  trace_config->WriteFormattedLine("core", "CommonConfig: %s", GetRootDirectory(commonConfigRootIndex).Get());

  trace_config->WriteFormattedLine("core", "CommonInstall: %s", GetRootDirectory(commonInstallRootIndex).Get());
}

unsigned SessionImpl::GetNumberOfTEXMFRoots()
{
  unsigned n = static_cast<unsigned>(rootDirectories.size());

  MIKTEX_ASSERT(n > 1);

  if (n <= 1)
  {
    MIKTEX_UNEXPECTED();
  }

  // the MPM root directory doesn't count
  return n - 1;
}

PathName SessionImpl::GetRootDirectory(unsigned r)
{
  unsigned n = GetNumberOfTEXMFRoots();
  if (r == INVALID_ROOT_INDEX || r >= n)
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }
  return rootDirectories[r].get_Path();
}

bool SessionImpl::IsCommonRootDirectory(unsigned r)
{
  unsigned n = GetNumberOfTEXMFRoots();
  if (r == INVALID_ROOT_INDEX || r >= n)
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }
  return rootDirectories[r].IsCommon();
}


unsigned SessionImpl::GetMpmRoot()
{
  return MPM_ROOT;
}

unsigned SessionImpl::GetInstallRoot()
{
  if (IsAdminMode())
  {
    return GetCommonInstallRoot();
  }
  else
  {
    return GetUserInstallRoot();
  }
}

unsigned SessionImpl::GetCommonInstallRoot()
{
  return commonInstallRootIndex;
}

unsigned SessionImpl::GetUserInstallRoot()
{
  return userInstallRootIndex;
}

unsigned SessionImpl::GetDistRoot()
{
  PathName myloc = GetMyLocation(true);
  RemoveDirectoryDelimiter(myloc.GetData());
  PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
  RemoveDirectoryDelimiter(internalBindir.GetData());
  PathName prefix;
  if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
  {
    return TryDeriveTEXMFRoot(prefix);
  }
  PathName bindir(MIKTEX_PATH_BIN_DIR);
  RemoveDirectoryDelimiter(bindir.GetData());
  if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
  {
    return TryDeriveTEXMFRoot(prefix);
  }
  return INVALID_ROOT_INDEX;
}

void SessionImpl::SaveRootDirectories(
#if defined(MIKTEX_WINDOWS)
  bool noRegistry
#endif
  )
{
#if ! defined(MIKTEX_WINDOWS)
  bool noRegistry = true;
#endif
  MIKTEX_ASSERT(!IsMiKTeXDirect());
  StartupConfig startupConfig;
  startupConfig.config =
    (IsMiKTeXPortable()
      ? MiKTeXConfiguration::Portable
      : MiKTeXConfiguration::Regular);
  unsigned n = GetNumberOfTEXMFRoots();
  startupConfig.commonRoots.reserve(n * 30);
  startupConfig.userRoots.reserve(n * 30);
  for (unsigned idx = 0; idx < n; ++idx)
  {
    const RootDirectory rootDirectory = this->rootDirectories[idx];
    if (rootDirectory.IsCommon())
    {
      if (idx == commonDataRootIndex
	|| idx == commonConfigRootIndex
	|| idx == commonInstallRootIndex)
      {
	// implicitly defined
	continue;
      }
      if (!startupConfig.commonRoots.empty())
      {
	startupConfig.commonRoots += PATH_DELIMITER;
      }
      startupConfig.commonRoots += rootDirectory.get_UnexpandedPath().Get();
    }
    else
    {
      if (idx == userDataRootIndex
	|| idx == userConfigRootIndex
	|| idx == userInstallRootIndex)
      {
	// implicitly defined
	continue;
      }
      if (!startupConfig.userRoots.empty())
      {
	startupConfig.userRoots += PATH_DELIMITER;
      }
      startupConfig.userRoots += rootDirectory.get_UnexpandedPath().Get();
    }
  }
  if (commonInstallRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.commonInstallRoot =
      this->rootDirectories[commonInstallRootIndex].get_UnexpandedPath();
  }
  if (userInstallRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.userInstallRoot = this->rootDirectories[userInstallRootIndex].get_UnexpandedPath();
  }
  if (commonDataRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.commonDataRoot = this->rootDirectories[commonDataRootIndex].get_UnexpandedPath();
  }
  if (userDataRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.userDataRoot = this->rootDirectories[userDataRootIndex].get_UnexpandedPath();
  }
  if (commonConfigRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.commonConfigRoot = this->rootDirectories[commonConfigRootIndex].get_UnexpandedPath();
  }
  if (userConfigRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig.userConfigRoot = this->rootDirectories[userConfigRootIndex].get_UnexpandedPath();
  }
  if (IsAdminMode())
  {
    PathName commonStartupConfigFile;
    bool haveCommonStartupConfigFile = FindStartupConfigFile(true, commonStartupConfigFile);
    if (haveCommonStartupConfigFile || noRegistry)
    {
      WriteStartupConfigFile(true, startupConfig);
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      WriteRegistry(true, startupConfig);
#else
      UNIMPLEMENTED();
#endif
    }
  }
  else
  {
    PathName userStartupConfigFile;
    bool haveUserStartupConfigFile = FindStartupConfigFile(false, userStartupConfigFile);
    if (haveUserStartupConfigFile || noRegistry)
    {
      WriteStartupConfigFile(false, startupConfig);
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      WriteRegistry(false, startupConfig);
#else
      UNIMPLEMENTED();
#endif
    }
  }
  time_t now = time(0);
  string nowStr = std::to_string(now);
  if (IsAdminMode())
  {
    SetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, nowStr.c_str());
  }
  else
  {
    SetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, nowStr.c_str());
  }
}

void SessionImpl::RegisterRootDirectories(const string & roots)
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (UseLocalServer())
  {
    ConnectToServer();
    HResult hr = localServer.pSession->RegisterRootDirectories(_bstr_t(roots.c_str()));
    if (hr.Failed())
    {
      MiKTeXSessionLib::ErrorInfo errorInfo;
      HResult hr2 = localServer.pSession->GetErrorInfo(&errorInfo);
      if (hr2.Failed())
      {
	MIKTEX_FATAL_ERROR_2(T_("sessionsvc failed for some reason."), "hr", hr.GetText());
      }
      AutoSysString a(errorInfo.message);
      AutoSysString b(errorInfo.info);
      AutoSysString c(errorInfo.sourceFile);
      Session::FatalMiKTeXError(string(WU_(errorInfo.message)), MiKTeXException::KVMAP(string(WU_(errorInfo.info))), SourceLocation("", string(WU_(errorInfo.sourceFile)), errorInfo.sourceLine));
    }
    return;
  }
#endif

  StartupConfig startupConfig;
  if (IsAdminMode())
  {
    startupConfig.commonRoots = roots;
  }
  else
  {
    startupConfig.userRoots = roots;
  }
  RegisterRootDirectoriesOptionSet options;
#if defined(MIKTEX_WINDOWS)
  // FIXME: should be: NO_REGISTRY ? false : true
  if (GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_NO_REGISTRY, NO_REGISTRY ? true : false))
  {
    options += RegisterRootDirectoriesOption::NoRegistry;
  }
#endif
  RegisterRootDirectories(startupConfig, options);
}

void SessionImpl::RegisterRootDirectories(const StartupConfig & startupConfig, RegisterRootDirectoriesOptionSet options)
{
  if (IsMiKTeXDirect())
  {
    MIKTEX_UNEXPECTED();
  }

  // clear the search path cache
  ClearSearchVectors();

  triMiKTeXDirect = TriState::Undetermined;

  StartupConfig startupConfig_ = startupConfig;

  if (startupConfig_.commonInstallRoot.Empty() && commonInstallRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.commonInstallRoot = GetRootDirectory(commonInstallRootIndex);
  }
  if (startupConfig_.commonDataRoot.Empty() && commonDataRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.commonDataRoot = GetRootDirectory(commonDataRootIndex);
  }
  if (startupConfig_.commonConfigRoot.Empty() && commonConfigRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.commonConfigRoot = GetRootDirectory(commonConfigRootIndex);
  }

  if (startupConfig_.userInstallRoot.Empty() && userInstallRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.userInstallRoot = GetRootDirectory(userInstallRootIndex);
  }

  if (startupConfig_.userDataRoot.Empty() && userDataRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.userDataRoot = GetRootDirectory(userDataRootIndex);
  }

  if (startupConfig_.userConfigRoot.Empty() && userConfigRootIndex != INVALID_ROOT_INDEX)
  {
    startupConfig_.userConfigRoot = GetRootDirectory(userConfigRootIndex);
  }

  MergeStartupConfig(startupConfig_, DefaultConfig());

  InitializeRootDirectories(startupConfig_);

  if (!options[RegisterRootDirectoriesOption::Temporary])
  {
    // save the information
#if defined(MIKTEX_WINDOWS)
    SaveRootDirectories(options[RegisterRootDirectoriesOption::NoRegistry]);
#else
    SaveRootDirectories();
#endif
  }
}

unsigned SessionImpl::GetDataRoot()
{
  if (IsAdminMode())
  {
    return GetCommonDataRoot();
  }
  else
  {
    return GetUserDataRoot();
  }
}

unsigned SessionImpl::GetCommonDataRoot()
{
  return commonDataRootIndex;
}

unsigned SessionImpl::GetUserDataRoot()
{
  return userDataRootIndex;
}

unsigned SessionImpl::GetConfigRoot()
{
  if (IsAdminMode())
  {
    return GetCommonConfigRoot();
  }
  else
  {
    return GetUserConfigRoot();
  }
}

unsigned SessionImpl::GetCommonConfigRoot()
{
  return commonConfigRootIndex;
}

unsigned SessionImpl::GetUserConfigRoot()
{
  return userConfigRootIndex;
}

bool SessionImpl::IsTeXMFReadOnly(unsigned r)
{
  return !IsMiKTeXPortable() && ((IsMiKTeXDirect() && r == GetInstallRoot()) || (rootDirectories[r].IsCommon() && !IsAdminMode()));
}

bool SessionImpl::FindFilenameDatabase(unsigned r, PathName & path)
{
  if (!(r < GetNumberOfTEXMFRoots() || r == MPM_ROOT))
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }

  vector<PathName> fndbFiles = GetFilenameDatabasePathNames(r);

  for (const PathName & p : GetFilenameDatabasePathNames(r))
  {
    if (File::Exists(p))
    {
      path = p;
      return true;
    }
  }

  return false;
}

PathName SessionImpl::GetFilenameDatabasePathName(unsigned r)
{
  return GetFilenameDatabasePathNames(r)[0];
}

vector<PathName> SessionImpl::GetFilenameDatabasePathNames(unsigned r)
{
  vector<PathName> result;

  if (!IsMiKTeXPortable())
  {
    // preferred pathname
    PathName path = rootDirectories[r].get_Path();
    if (rootDirectories[r].IsCommon())
    {
      path = GetSpecialPath(SpecialPath::CommonDataRoot);
    }
    else
    {
      path = GetSpecialPath(SpecialPath::UserDataRoot);
    }
    path /= GetRelativeFilenameDatabasePathName(r);
    result.push_back(path);
  }

  PathName path;

  // alternative pathname
  if (r == MPM_ROOT)
  {
    // INSTALL\miktex\conig\mpm.fndb
    if (GetInstallRoot() == INVALID_ROOT_INDEX)
    {
      MIKTEX_UNEXPECTED();
    }
    path.Set(rootDirectories[GetInstallRoot()].get_Path(), MIKTEX_PATH_MPM_FNDB);
  }
  else
  {
    // ROOT\miktex\conig\texmf.fndb
    path.Set(rootDirectories[r].get_Path(), MIKTEX_PATH_TEXMF_FNDB);
  }
  result.push_back(path);

  return result;
}

PathName SessionImpl::GetMpmDatabasePathName()
{
  return GetFilenameDatabasePathName(MPM_ROOT);
}

PathName SessionImpl::GetMpmRootPath()
{
  return MPM_ROOT_PATH;
}

PathName SessionImpl::GetRelativeFilenameDatabasePathName(unsigned r)
{
  string fndbFileName = MIKTEX_PATH_FNDB_DIR;
  fndbFileName += PathName::DirectoryDelimiter;
  PathName root(rootDirectories[r].get_Path());
  root.TransformForComparison();
  MD5Builder md5Builder;
  md5Builder.Update(root.Get(), root.GetLength());
  md5Builder.Final();
  fndbFileName += md5Builder.GetMD5().ToString();
  fndbFileName += MIKTEX_FNDB_FILE_SUFFIX;
  return fndbFileName;
}

shared_ptr<FileNameDatabase> SessionImpl::GetFileNameDatabase(unsigned r)
{
  return GetFileNameDatabase(r, TriState::Undetermined);
}

shared_ptr<FileNameDatabase> SessionImpl::GetFileNameDatabase(unsigned r, TriState triReadOnly)
{
  if (r != MPM_ROOT && r >= GetNumberOfTEXMFRoots())
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }

  bool readOnly;

  if (triReadOnly == TriState::True)
  {
    readOnly = true;
  }
  else if (triReadOnly == TriState::False)
  {
    readOnly = false;
  }
  else                  // triReadOnly == TriState::Undetermined
  {
    readOnly = IsTeXMFReadOnly(r);
  }

  lock_guard<mutex> lockGuard(fndbMutex);

  RootDirectory & root = rootDirectories[r];

  shared_ptr<FileNameDatabase> fndb = root.GetFndb();
  if (fndb != nullptr)
  {
    if (triReadOnly == TriState::False && fndb->IsInvariable())
    {
      // we say we indend to modify the fndb; but the fndb is opened readonly
      // => we have to reload the database
      fndb = nullptr;
      if (!UnloadFilenameDatabaseInternal_nolock(r, false))
      {
        return nullptr;
      }
    }
    else
    {
      return fndb;
    }
  }

#if 0
  if (root.get_NoFndb())
  {
    // don't try to load the file name database
    return nullptr;
  }
#endif

  PathName fqFndbFileName;

  bool fndbFileExists = FindFilenameDatabase(r, fqFndbFileName);

  if (!fndbFileExists)
  {
#if 0
    TraceError(T_("there is no fndb file for %s"), Q_(root.get_Path()));
#endif
#if 0
    root.set_NoFndb(true);
#endif
    return nullptr;
  }

  trace_fndb->WriteFormattedLine("core", T_("loading fndb%s: %s"), (readOnly ? T_(" read-only") : ""), fqFndbFileName.Get());

  shared_ptr<FileNameDatabase> pFndb = FileNameDatabase::Create(fqFndbFileName.Get(), root.get_Path().Get(), readOnly);

  root.SetFndb(pFndb);

  return pFndb;
}

shared_ptr<FileNameDatabase> SessionImpl::GetFileNameDatabase(const char * lpszPath)
{
  unsigned root = TryDeriveTEXMFRoot(lpszPath);
  if (root == INVALID_ROOT_INDEX)
  {
    return nullptr;
  }
  return GetFileNameDatabase(root, TriState::Undetermined);
}

unsigned SessionImpl::TryDeriveTEXMFRoot(const PathName & path)
{
  if (!Utils::IsAbsolutePath(path.Get()))
  {
#if FIND_FILE_PREFER_RELATIVE_PATH_NAMES
    return INVALID_ROOT_INDEX;
#else
    INVALID_ARGUMENT("path", path.ToString());
#endif
  }

  if (IsMpmFile(path.Get()))
  {
    return MPM_ROOT;
  }

  unsigned rootDirectoryIndex = INVALID_ROOT_INDEX;

  unsigned n = GetNumberOfTEXMFRoots();

  for (unsigned idx = 0; idx < n; ++idx)
  {
    PathName pathRoot = GetRootDirectory(idx);
    size_t rootlen = pathRoot.GetLength();
    if (PathName::Compare(pathRoot, path, rootlen) == 0 && (pathRoot.EndsWithDirectoryDelimiter() || path[rootlen] == 0 || IsDirectoryDelimiter(path[rootlen])))
    {
      if (rootDirectoryIndex == INVALID_ROOT_INDEX)
      {
	rootDirectoryIndex = idx;
      }
      else if (GetRootDirectory(rootDirectoryIndex).GetLength() < rootlen)
      {
	rootDirectoryIndex = idx;
      }
    }
  }

  return rootDirectoryIndex;
}

unsigned SessionImpl::DeriveTEXMFRoot(const PathName & path)
{
  unsigned root = TryDeriveTEXMFRoot(path);
  if (root == INVALID_ROOT_INDEX)
  {
    MIKTEX_UNEXPECTED();
  }
  return root;
}

bool SessionImpl::UnloadFilenameDatabaseInternal(unsigned r, bool remove)
{
  lock_guard<mutex> lockGuard(fndbMutex);
  return UnloadFilenameDatabaseInternal_nolock(r, remove);
}

bool SessionImpl::UnloadFilenameDatabaseInternal_nolock(unsigned r, bool remove)
{
  trace_fndb->WriteFormattedLine("core", T_("going to unload file name database %u"), r);

  shared_ptr<FileNameDatabase> fndb = rootDirectories[r].GetFndb();
  if (fndb != nullptr)
  {
    // check the reference count
    if (fndb.use_count() > 2)
    {
      trace_fndb->WriteFormattedLine("core", T_("cannot unload fndb #%u: still in use (%u)"), r, fndb.use_count());
      return false;
    }

    // release the database file
    fndb = nullptr;
    rootDirectories[r].SetFndb(nullptr);
  }

  if (remove && r < GetNumberOfTEXMFRoots())
  {
    // remove the database file
    PathName path = GetFilenameDatabasePathName(r);
    if (File::Exists(path))
    {
      File::Delete(path, { FileDeleteOption::TryHard });
    }
  }

  return true;
}

bool SessionImpl::UnloadFilenameDatabase()
{
  bool done = true;

  for (unsigned r = 0; r < rootDirectories.size(); ++r)
  {
    if (!UnloadFilenameDatabaseInternal(r, false))
    {
      done = false;
    }
  }

  return done;
}

bool SessionImpl::IsTEXMFFile(const PathName & path, PathName & relPath, unsigned & rootIndex)
{
  for (unsigned r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    PathName pathRoot = GetRootDirectory(r);
    size_t cchRoot = pathRoot.GetLength();
    if (PathName::Compare(pathRoot, path, cchRoot) == 0 && (path[cchRoot] == 0 || IsDirectoryDelimiter(path[cchRoot])))
    {
      const char * lpsz = &path[cchRoot];
      if (IsDirectoryDelimiter(*lpsz))
      {
        ++lpsz;
      }
      relPath = lpsz;
      rootIndex = r;
      return true;
    }
  }
  return false;
}

unsigned SessionImpl::SplitTEXMFPath(const PathName & path, PathName & root, PathName & relative)
{
  for (unsigned r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    PathName rootDir = GetRootDirectory(r);
    size_t rootDirLen = rootDir.GetLength();
    if (PathName::Compare(rootDir, path, rootDirLen) == 0 && (path[rootDirLen] == 0 || IsDirectoryDelimiter(path[rootDirLen])))
    {
      CopyString2(root.GetData(), BufferSizes::MaxPath, rootDir.Get(), rootDirLen);
      const char * lpsz = &path[0] + rootDirLen;
      if (IsDirectoryDelimiter(*lpsz))
      {
	++lpsz;
      }
      relative = lpsz;
      return r;
    }
  }

  return INVALID_ROOT_INDEX;
}

bool SessionImpl::IsManagedRoot(unsigned root)
{
  return
    root == GetUserInstallRoot() ||
    root == GetUserConfigRoot() ||
    root == GetUserDataRoot() ||
    root == GetCommonInstallRoot() ||
    root == GetCommonConfigRoot() ||
    root == GetCommonDataRoot();
}

bool SessionImpl::IsMpmFile(const char * lpszPath)
{
  return (PathName::Compare(MPM_ROOT_PATH, lpszPath, static_cast<unsigned long>(MPM_ROOT_PATH_LEN)) == 0
    && (lpszPath[MPM_ROOT_PATH_LEN] == 0 || IsDirectoryDelimiter(lpszPath[MPM_ROOT_PATH_LEN])));
}

bool Utils::IsMiKTeXDirectRoot(const PathName & root)
{
  PathName path(root);
  path /= MIKTEXDIRECT_PREFIX_DIR;
  path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
  if (!File::Exists(path))
  {
    return false;
  }
  FileAttributeSet attributes = File::GetAttributes(path);
  if (!attributes[FileAttribute::ReadOnly])
  {
    return false;
  }
  unique_ptr<Cfg> pcfg(Cfg::Create());
  pcfg->Read(path);
  string str;
  return pcfg->TryGetValue("Auto", "Config", str) && str == "Direct";
}
