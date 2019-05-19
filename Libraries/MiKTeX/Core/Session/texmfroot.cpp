/* texmfroot.cpp: managing TEXMF root directories

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

#include <mutex>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Environment>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Core/RootDirectoryInfo>

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
#  include "miktex/Core/win/HResult.h"
#  include "miktex/Core/win/winAutoResource.h"
#endif

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

// index of the hidden MPM root
#define MPM_ROOT GetNumberOfTEXMFRoots()

namespace {
  mutex fndbMutex;
}

static PathName ExpandEnvironmentVariables(const PathName& toBeExpanded)
{
  const char BEGIN = '<';
  const char END = '>';
  string expansion;
  for (const char* lpsz = toBeExpanded.GetData(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == BEGIN)
    {
      string valueName;
      for (lpsz += 1; *lpsz != 0 && *lpsz != END; ++lpsz)
      {
        valueName += *lpsz;
      }
      if (*lpsz != END || valueName.empty())
      {
        MIKTEX_UNEXPECTED();
      }
      string value;
      if (!Utils::GetEnvironmentString(valueName, value))
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

unsigned SessionImpl::RegisterRootDirectory(const PathName& root, RootDirectoryInfo::Purpose purpose, ConfigurationScope scope, bool other, bool review)
{
  unsigned idx;
  for (idx = 0; idx < rootDirectories.size(); ++idx)
  {
    if (review)
    {
      if (Utils::IsParentDirectoryOf(rootDirectories[idx].get_Path(), root))
      {
        MIKTEX_FATAL_ERROR_3(T_("Invalid root directory."), T_("The requested root directory ({requested}) is a sub-directory of another root directory ({other})."), "requested", root.ToDisplayString(), "other", rootDirectories[idx].get_Path().ToDisplayString());
      }
      if (Utils::IsParentDirectoryOf(root, rootDirectories[idx].get_Path()))
      {
        MIKTEX_FATAL_ERROR_3(T_("Invalid root directory."), T_("The requested root directory ({requested}) is the parent directory of another root directory ({other})."), "requested", root.ToDisplayString(), "other", rootDirectories[idx].get_Path().ToDisplayString());
      }
    }
    if (root == rootDirectories[idx].get_UnexpandedPath())
    {
      // already registered
      if (scope == ConfigurationScope::Common && !rootDirectories[idx].IsCommon())
      {
        trace_config->WriteLine("core", fmt::format(T_("now a common TEXMF root: {0}"), root));
        rootDirectories[idx].set_Common(scope == ConfigurationScope::Common);
      }
      if (other && !rootDirectories[idx].IsOther())
      {
        trace_config->WriteLine("core", fmt::format(T_("now a foreign TEXMF root: {0}"), root));
        rootDirectories[idx].set_Common(scope == ConfigurationScope::Common);
      }
      rootDirectories[idx].purposes += purpose;;
      return idx;
    }
  }
  trace_config->WriteLine("core", fmt::format(T_("registering {0} TEXMF root: {1}"), scope == ConfigurationScope::Common ? "common" : "user", root));
  RootDirectoryInternals rootDirectory(root, ExpandEnvironmentVariables(root));
  rootDirectory.purposes += purpose;
  rootDirectory.set_Common(scope == ConfigurationScope::Common);
  rootDirectory.set_Other(other);
  rootDirectories.reserve(10);
  rootDirectories.push_back(rootDirectory);
  return idx;
}

void SessionImpl::InitializeRootDirectories(const StartupConfig& startupConfig, bool review)
{
  rootDirectories.clear();

  commonInstallRootIndex = INVALID_ROOT_INDEX;
  userInstallRootIndex = INVALID_ROOT_INDEX;
  commonDataRootIndex = INVALID_ROOT_INDEX;
  userDataRootIndex = INVALID_ROOT_INDEX;
  commonConfigRootIndex = INVALID_ROOT_INDEX;
  userConfigRootIndex = INVALID_ROOT_INDEX;

  if (!IsAdminMode())
  {
    // UserConfig
    if (!startupConfig.userConfigRoot.Empty())
    {
      userConfigRootIndex = RegisterRootDirectory(startupConfig.userConfigRoot, RootDirectoryInfo::Purpose::Config, ConfigurationScope::User, false, review);
    }

    // UserData
    if (!startupConfig.userDataRoot.Empty())
    {
      userDataRootIndex = RegisterRootDirectory(startupConfig.userDataRoot, RootDirectoryInfo::Purpose::Data, ConfigurationScope::User, false, review);
    }

    // UserRoots
    for (const string& root : StringUtil::Split(startupConfig.userRoots, PathName::PathNameDelimiter))
    {
      if (!root.empty())
      {
        RegisterRootDirectory(root, RootDirectoryInfo::Purpose::Generic, ConfigurationScope::User, false, review);
      }
    }

    // UserInstall
    if (!startupConfig.userInstallRoot.Empty())
    {
      userInstallRootIndex = RegisterRootDirectory(startupConfig.userInstallRoot, RootDirectoryInfo::Purpose::Install, ConfigurationScope::User, false, review);
    }
  }

  // CommonConfig
  if (!startupConfig.commonConfigRoot.Empty())
  {
    commonConfigRootIndex = RegisterRootDirectory(startupConfig.commonConfigRoot, RootDirectoryInfo::Purpose::Config, ConfigurationScope::Common, false, review);
  }

  // CommonData
  if (!startupConfig.commonDataRoot.Empty())
  {
    commonDataRootIndex = RegisterRootDirectory(startupConfig.commonDataRoot, RootDirectoryInfo::Purpose::Data, ConfigurationScope::Common, false, review);
  }

  // CommonRoots
  for (const string& root : StringUtil::Split(startupConfig.commonRoots, PathName::PathNameDelimiter))
  {
    if (!root.empty())
    {
      RegisterRootDirectory(root, RootDirectoryInfo::Purpose::Generic, ConfigurationScope::Common, false, review);
    }
  }

  // CommonInstall
  if (!startupConfig.commonInstallRoot.Empty())
  {
    commonInstallRootIndex = RegisterRootDirectory(startupConfig.commonInstallRoot, RootDirectoryInfo::Purpose::Install, ConfigurationScope::Common, false, review);
  }

  if (!IsAdminMode())
  {
    // OtherUserRoots
    for (const string& root : StringUtil::Split(startupConfig.otherUserRoots, PathName::PathNameDelimiter))
    {
      if (!root.empty())
      {
        RegisterRootDirectory(root, RootDirectoryInfo::Purpose::Generic, ConfigurationScope::User, true, review);
      }
    }
  }

  // OtherCommonRoots
  for (const string& root : StringUtil::Split(startupConfig.otherCommonRoots, PathName::PathNameDelimiter))
  {
    if (!root.empty())
    {
      RegisterRootDirectory(root, RootDirectoryInfo::Purpose::Generic, ConfigurationScope::Common, true, review);
    }
  }

  if (rootDirectories.empty())
  {
    MIKTEX_UNEXPECTED();
  }

  if (!IsAdminMode())
  {
    if (userDataRootIndex == INVALID_ROOT_INDEX)
    {
      userDataRootIndex = 0;
    }
    if (userConfigRootIndex == INVALID_ROOT_INDEX)
    {
      userConfigRootIndex = userDataRootIndex;
    }
    if (userInstallRootIndex == INVALID_ROOT_INDEX)
    {
      userInstallRootIndex = userConfigRootIndex;
    }
  }

  if (commonDataRootIndex == INVALID_ROOT_INDEX)
  {
    commonDataRootIndex = 0;
  }
  
  if (commonConfigRootIndex == INVALID_ROOT_INDEX)
  {
    commonConfigRootIndex = commonDataRootIndex;
  }

  if (commonInstallRootIndex == INVALID_ROOT_INDEX)
  {
    commonInstallRootIndex = commonConfigRootIndex;
  }

  RegisterRootDirectory(MPM_ROOT_PATH, RootDirectoryInfo::Purpose::Generic, IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, false, false);

  if (!IsAdminMode())
  {
    trace_config->WriteLine("core", fmt::format("UserData: {}", GetRootDirectoryPath(userDataRootIndex)));
    trace_config->WriteLine("core", fmt::format("UserConfig: {}", GetRootDirectoryPath(userConfigRootIndex)));
    trace_config->WriteLine("core", fmt::format("UserInstall: {}", GetRootDirectoryPath(userInstallRootIndex)));
  }

  trace_config->WriteLine("core", fmt::format("CommonData: {}", GetRootDirectoryPath(commonDataRootIndex)));
  trace_config->WriteLine("core", fmt::format("CommonConfig: {}", GetRootDirectoryPath(commonConfigRootIndex)));
  trace_config->WriteLine("core", fmt::format("CommonInstall: {}", GetRootDirectoryPath(commonInstallRootIndex)));
}

vector<RootDirectoryInfo> SessionImpl::GetRootDirectories()
{
  vector<RootDirectoryInfo> result;
  MIKTEX_ASSERT(rootDirectories.size() > 1);
  if (rootDirectories.size() <= 1)
  {
    MIKTEX_UNEXPECTED();
  }
  for (size_t r = 0; r < rootDirectories.size() - 1; ++r)
  {
    result.push_back(rootDirectories[r]);
  }
  return result;
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

PathName SessionImpl::GetRootDirectoryPath(unsigned r)
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

bool SessionImpl::IsOtherRootDirectory(unsigned r)
{
  unsigned n = GetNumberOfTEXMFRoots();
  if (r == INVALID_ROOT_INDEX || r >= n)
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }
  return rootDirectories[r].IsOther();
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

pair<bool, PathName> SessionImpl::TryGetDistRootDirectory()
{
#if defined(MIKTEX_WINDOWS)
  PathName myloc = GetMyLocation(true);
  RemoveDirectoryDelimiter(myloc.GetData());
  PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
  RemoveDirectoryDelimiter(internalBindir.GetData());
  PathName prefix;
  if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
  {
    return make_pair(true, prefix);
  }
  PathName bindir(MIKTEX_PATH_BIN_DIR);
  RemoveDirectoryDelimiter(bindir.GetData());
  if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
  {
    return make_pair(true, prefix);
  }
  return make_pair(false, PathName());
#else
  return make_pair(true, GetMyPrefix(true) / MIKTEX_DIST_DIR);
#endif
}

PathName SessionImpl::GetDistRootDirectory()
{
  auto result = TryGetDistRootDirectory();
  if (!result.first)
  {
    MIKTEX_UNEXPECTED();
  }
  return result.second;
}

void SessionImpl::ReregisterRootDirectories(const string& roots, bool other)
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (UseLocalServer())
  {
    if (other)
    {
      MIKTEX_UNEXPECTED();
    }
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
      Session::FatalMiKTeXError(string(WU_(errorInfo.message)), "", "", "", MiKTeXException::KVMAP("info", string(WU_(errorInfo.info))), SourceLocation("", string(WU_(errorInfo.sourceFile)), errorInfo.sourceLine));
    }
    return;
  }
#endif

  StartupConfig startupConfig;
  if (IsAdminMode() || IsMiKTeXPortable())
  {
    if (other)
    {
      startupConfig.otherCommonRoots = roots;
    }
    else
    {
      startupConfig.commonRoots = roots;
    }
  }
  else
  {
    if (other)
    {
      startupConfig.otherUserRoots = roots;
    }
    else
    {
      startupConfig.userRoots = roots;
    }
  }
  RegisterRootDirectoriesOptionSet options;
  options += RegisterRootDirectoriesOption::Review;
#if defined(MIKTEX_WINDOWS)
  // FIXME: should be: NO_REGISTRY ? false : true
  if (IsMiKTeXPortable() || GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_NO_REGISTRY, NO_REGISTRY ? true : false).GetBool())
  {
    options += RegisterRootDirectoriesOption::NoRegistry;
  }
#endif
  RegisterRootDirectories(startupConfig, options);
}

void SessionImpl::RegisterRootDirectory(const PathName& path, bool other)
{
  vector<string> toBeRegistered;
  for (size_t r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    const RootDirectoryInternals& root = rootDirectories[r];
    bool skipit = other && !root.IsOther() || !other && root.IsOther();
    skipit = skipit || (IsAdminMode() && !root.IsCommon());
    skipit = skipit || (!IsAdminMode() && root.IsCommon());
    skipit = skipit || root.IsManaged();
    if (!skipit)
    {
      toBeRegistered.push_back(rootDirectories[r].path.ToString());
    }
  }
  toBeRegistered.push_back(path.ToString());
  ReregisterRootDirectories(StringUtil::Flatten(toBeRegistered, PathName::PathNameDelimiter), other);
}

void SessionImpl::UnregisterRootDirectory(const PathName& path, bool other)
{
  vector<string> toBeRegistered;
  bool found = false;
  for (size_t r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    const RootDirectoryInternals& root = rootDirectories[r];
    bool skipit = other && !root.IsOther() || !other && root.IsOther();
    skipit = skipit || (IsAdminMode() && !root.IsCommon());
    skipit = skipit || (!IsAdminMode() && root.IsCommon());
    skipit = skipit || root.IsManaged();
    if (!skipit)
    {
      skipit = root.path == path;
      if (skipit)
      {
        found = true;
      }
      else
      {
        toBeRegistered.push_back(rootDirectories[r].path.ToString());
      }
    }
  }
  if (!found)
  {
    MIKTEX_UNEXPECTED();
  }
  ReregisterRootDirectories(StringUtil::Flatten(toBeRegistered, PathName::PathNameDelimiter), other);
}

void SessionImpl::RegisterRootDirectories(const StartupConfig& partialStartupConfig, RegisterRootDirectoriesOptionSet options)
{
  if (IsMiKTeXDirect())
  {
    MIKTEX_UNEXPECTED();
  }

  // clear the search path cache
  ClearSearchVectors();

  StartupConfig newStartupConfig = partialStartupConfig;
  newStartupConfig.config = initStartupConfig.config;

  if (newStartupConfig.commonInstallRoot.Empty() && commonInstallRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.commonInstallRoot = GetRootDirectoryPath(commonInstallRootIndex);
  }

  if (newStartupConfig.commonDataRoot.Empty() && commonDataRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.commonDataRoot = GetRootDirectoryPath(commonDataRootIndex);
  }

  if (newStartupConfig.commonConfigRoot.Empty() && commonConfigRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.commonConfigRoot = GetRootDirectoryPath(commonConfigRootIndex);
  }

  if (newStartupConfig.userInstallRoot.Empty() && userInstallRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.userInstallRoot = GetRootDirectoryPath(userInstallRootIndex);
  }

  if (newStartupConfig.userDataRoot.Empty() && userDataRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.userDataRoot = GetRootDirectoryPath(userDataRootIndex);
  }

  if (newStartupConfig.userConfigRoot.Empty() && userConfigRootIndex != INVALID_ROOT_INDEX)
  {
    newStartupConfig.userConfigRoot = GetRootDirectoryPath(userConfigRootIndex);
  }

  MergeStartupConfig(newStartupConfig, DefaultConfig());

  try
  {
    InitializeRootDirectories(newStartupConfig, options[RegisterRootDirectoriesOption::Review]);
  }
  catch (const MiKTeXException&)
  {
    InitializeRootDirectories(initStartupConfig, false);
    throw;
  }

  if (!options[RegisterRootDirectoriesOption::Temporary])
  {
    SaveStartupConfig(newStartupConfig, options);
  }
}

void SessionImpl::MoveRootDirectory(unsigned r, int dir)
{
  MIKTEX_ASSERT(dir == -1 || dir == 1);
  bool up = dir < 0;
  unsigned n = GetNumberOfTEXMFRoots();
  if (r == INVALID_ROOT_INDEX || r >= n)
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }
  const RootDirectoryInternals& root = rootDirectories[r];
  bool canMove = !root.IsManaged();
  canMove = canMove && (!IsAdminMode() || root.IsCommon());
  canMove = canMove && (IsAdminMode() || !root.IsCommon());
  if (up)
  {
    canMove = canMove && r > 0;
    canMove = canMove && !rootDirectories[r - 1].IsManaged();
  }
  else
  {
    canMove = canMove && r < n - 1;
    canMove = canMove && !rootDirectories[static_cast<size_t>(r) + 1].IsManaged();
  }  
  if (!canMove)
  {
    MIKTEX_UNEXPECTED();
  }
  vector<RootDirectoryInternals> newRoots = rootDirectories;
  if (up)
  {
    swap(newRoots[r], newRoots[r - 1]);
  }
  else
  {
    swap(newRoots[r], newRoots[static_cast<size_t>(r) + 1]);
  }
  vector<string> toBeRegistered;
  for (unsigned r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    const RootDirectoryInternals& root = newRoots[r];
    if (!root.IsManaged() && ((IsAdminMode() && root.IsCommon()) || (!IsAdminMode() && !root.IsCommon())))
    {
      toBeRegistered.push_back(root.path.ToString());
    }
  }
  ReregisterRootDirectories(StringUtil::Flatten(toBeRegistered, PathName::PathNameDelimiter), false);
}

void SessionImpl::MoveRootDirectoryUp(unsigned r)
{
  MoveRootDirectory(r, -1);
}

void SessionImpl::MoveRootDirectoryDown(unsigned r)
{
  MoveRootDirectory(r, 1);
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
  if (r == MPM_ROOT)
  {
    return true;
  }
  if (rootDirectories[r].IsOther())
  {
    return true;
  }
  if (IsMiKTeXPortable())
  {
    return false;
  }
  return
     ((IsMiKTeXDirect() && r == GetInstallRoot())
      || (rootDirectories[r].IsCommon() && !IsAdminMode()));
}

bool SessionImpl::FindFilenameDatabase(unsigned r, PathName& path)
{
  if (!(r < GetNumberOfTEXMFRoots() || r == MPM_ROOT))
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }

  vector<PathName> fndbFiles = GetFilenameDatabasePathNames(r);

  for (const PathName& p : GetFilenameDatabasePathNames(r))
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
    path = rootDirectories[GetInstallRoot()].get_Path() / MIKTEX_PATH_MPM_FNDB;
  }
  else
  {
    // ROOT\miktex\conig\texmf.fndb
    path = rootDirectories[r].get_Path() / MIKTEX_PATH_TEXMF_FNDB;
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
  md5Builder.Update(root.GetData(), root.GetLength());
  md5Builder.Final();
  fndbFileName += md5Builder.GetMD5().ToString();
  fndbFileName += MIKTEX_FNDB_FILE_SUFFIX;
  return fndbFileName;
}

shared_ptr<FileNameDatabase> SessionImpl::GetFileNameDatabase(unsigned r)
{
  if (r != MPM_ROOT && r >= GetNumberOfTEXMFRoots())
  {
    INVALID_ARGUMENT("index", std::to_string(r));
  }

  lock_guard<mutex> lockGuard(fndbMutex);

  RootDirectoryInternals& root = rootDirectories[r];

  shared_ptr<FileNameDatabase> fndb = root.GetFndb();
  if (fndb != nullptr)
  {
    return fndb;
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
    TraceError(fmt::format(T_("there is no fndb file for {0}"), Q_(root.get_Path())));
#endif
#if 0
    root.set_NoFndb(true);
#endif
    return nullptr;
  }

  trace_fndb->WriteLine("core", fmt::format(T_("loading fndb: {0}"), fqFndbFileName));

  shared_ptr<FileNameDatabase> pFndb = FileNameDatabase::Create(fqFndbFileName, root.get_Path());

  root.SetFndb(pFndb);

  return pFndb;
}

shared_ptr<FileNameDatabase> SessionImpl::GetFileNameDatabase(const char* path)
{
  unsigned root = TryDeriveTEXMFRoot(path);
  if (root == INVALID_ROOT_INDEX)
  {
    return nullptr;
  }
  return GetFileNameDatabase(root);
}

unsigned SessionImpl::TryDeriveTEXMFRoot(const PathName& path)
{
  if (!Utils::IsAbsolutePath(path))
  {
#if FIND_FILE_PREFER_RELATIVE_PATH_NAMES
    return INVALID_ROOT_INDEX;
#else
    INVALID_ARGUMENT("path", path.ToString());
#endif
  }

  if (IsMpmFile(path.GetData()))
  {
    return MPM_ROOT;
  }

  unsigned rootDirectoryIndex = INVALID_ROOT_INDEX;

  unsigned n = GetNumberOfTEXMFRoots();

  for (unsigned idx = 0; idx < n; ++idx)
  {
    PathName pathRoot = GetRootDirectoryPath(idx);
    size_t rootlen = pathRoot.GetLength();
    if (PathName::Compare(pathRoot, path, rootlen) == 0 && (pathRoot.EndsWithDirectoryDelimiter() || path[rootlen] == 0 || IsDirectoryDelimiter(path[rootlen])))
    {
      if (rootDirectoryIndex == INVALID_ROOT_INDEX)
      {
        rootDirectoryIndex = idx;
      }
      else if (GetRootDirectoryPath(rootDirectoryIndex).GetLength() < rootlen)
      {
        rootDirectoryIndex = idx;
      }
    }
  }

  return rootDirectoryIndex;
}

unsigned SessionImpl::DeriveTEXMFRoot(const PathName& path)
{
  unsigned root = TryDeriveTEXMFRoot(path);
  if (root == INVALID_ROOT_INDEX)
  {
    MIKTEX_UNEXPECTED();
  }
  return root;
}

bool SessionImpl::UnloadFilenameDatabaseInternal(unsigned r, chrono::duration<double> minIdleTime)
{
  lock_guard<mutex> lockGuard(fndbMutex);
  return UnloadFilenameDatabaseInternal_nolock(r, minIdleTime);
}

bool SessionImpl::UnloadFilenameDatabaseInternal_nolock(unsigned r, chrono::duration<double> minIdleTime)
{
  shared_ptr<FileNameDatabase> fndb = rootDirectories[r].GetFndb();

  if (fndb != nullptr)
  {
    trace_fndb->WriteLine("core", fmt::format(T_("going to unload file name database #{0}"), r));

    // check the reference count
    if (fndb.use_count() > 2)
    {
      trace_fndb->WriteLine("core", fmt::format(T_("cannot unload fndb #{0}: still in use (use_count={1})"), r, fndb.use_count()));
      return false;
    }

    chrono::duration<double> idleTime = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - fndb->GetLastAccessTime());
    if (idleTime < minIdleTime)
    {
      trace_fndb->WriteLine("core", fmt::format(T_("cannot unload fndb #{0}: still in use (idle for {1:.4f} seconds)"), r, idleTime.count()));
      return false;
    }

    // release the database file
    fndb = nullptr;
    rootDirectories[r].SetFndb(nullptr);
  }

  return true;
}

bool SessionImpl::UnloadFilenameDatabase(chrono::duration<double> minIdleTime)
{
  bool done = true;

  for (unsigned r = 0; r < rootDirectories.size(); ++r)
  {
    if (!UnloadFilenameDatabaseInternal(r, minIdleTime))
    {
      done = false;
    }
  }

  return done;
}

bool SessionImpl::IsTEXMFFile(const PathName& path, PathName& relPath, unsigned& rootIndex)
{
  for (unsigned r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    PathName pathRoot = GetRootDirectoryPath(r);
    size_t cchRoot = pathRoot.GetLength();
    if (PathName::Compare(pathRoot, path, cchRoot) == 0 && (path[cchRoot] == 0 || IsDirectoryDelimiter(path[cchRoot])))
    {
      const char* lpsz = &path[cchRoot];
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

unsigned SessionImpl::SplitTEXMFPath(const PathName& path, PathName& root, PathName& relative)
{
  for (unsigned r = 0; r < GetNumberOfTEXMFRoots(); ++r)
  {
    PathName rootDir = GetRootDirectoryPath(r);
    size_t rootDirLen = rootDir.GetLength();
    if (PathName::Compare(rootDir, path, rootDirLen) == 0 && (path[rootDirLen] == 0 || IsDirectoryDelimiter(path[rootDirLen])))
    {
      root = rootDir;
      root[rootDirLen] = 0;
      const char* lpsz = &path[0] + rootDirLen;
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

bool SessionImpl::IsMpmFile(const char* lpszPath)
{
  return (PathName::Compare(MPM_ROOT_PATH, lpszPath, static_cast<unsigned long>(MPM_ROOT_PATH_LEN)) == 0
    && (lpszPath[MPM_ROOT_PATH_LEN] == 0 || IsDirectoryDelimiter(lpszPath[MPM_ROOT_PATH_LEN])));
}

bool Utils::IsMiKTeXDirectRoot(const PathName& root)
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
  unique_ptr<Cfg> cfg(Cfg::Create());
  cfg->Read(path);
  string str;
  return cfg->TryGetValueAsString("Auto", "Config", str) && str == "Direct";
}
