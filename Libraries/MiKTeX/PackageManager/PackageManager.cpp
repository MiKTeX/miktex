/* PackageManager.cpp: MiKTeX Package Manager

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "TpmParser.h"

#include "../WebServiceClients/Repository/repositoryRepositorySoapProxy.h"
#include "../WebServiceClients/Repository/RepositorySoap.nsmap"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

#define WEBSVCURL "http://api.miktex.org/Repository.asmx"

string PackageManagerImpl::proxyUser;
string PackageManagerImpl::proxyPassword;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
bool PackageManagerImpl::localServer = false;
#endif

class MyRepositorySoapProxy : public RepositorySoapProxy
{
public:
  MyRepositorySoapProxy()
  {
    this->soap_endpoint = WEBSVCURL;
  }
};

template<class Base> struct ClientInfo : public Base
{
  ClientInfo() :
    version_(MIKTEX_COMPONENT_VERSION_STR)
  {
#if 1
    for (const string & invoker : Process2::GetInvokerNames())
    {
      name_ += invoker;
      name_ += "/";
    }
    name_ += Utils::GetExeName();
    name_ += "/";
#endif
    name_ += "MPM";
    Base::Name = &name_;
    Base::Version = &version_;
  };
private:
  string name_;
  string version_;
};

void FatalSoapError(soap * pSoap, const SourceLocation & sourceLocation)
{
  if (soap_check_state(pSoap))
  {
    MIKTEX_UNEXPECTED();
  }
  else if (pSoap->error != SOAP_OK)
  {
    const char ** ppText = soap_faultstring(pSoap);
    string text;
    if (ppText != nullptr && *ppText != nullptr)
    {
      MIKTEX_ASSERT(Utils::IsUTF8(*ppText));
      text = *ppText;
    }
    const char ** ppDetail = soap_faultdetail(pSoap);
    string detail;
    if (ppDetail != nullptr && *ppDetail != nullptr)
    {
      MIKTEX_ASSERT(Utils::IsUTF8(*ppDetail));
      detail = *ppDetail;
    }
    Session::FatalMiKTeXError(text, detail, sourceLocation);
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

PackageManager::~PackageManager()
{
}

PackageManagerImpl::PackageManagerImpl(const PackageManager::InitInfo & initInfo) :
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, initInfo.traceCallback)),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM, initInfo.traceCallback)),
  pSession(Session::Get()),
  webSession(WebSession::Create(this))
{
  trace_mpm->WriteFormattedLine("libmpm", T_("initializing MPM library version %s"), MIKTEX_COMPONENT_VERSION_STR);
}

PackageManagerImpl::~PackageManagerImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception &)
  {
  }
}

shared_ptr<PackageManager> PackageManager::Create(const PackageManager::InitInfo & initInfo)
{
  return make_shared<PackageManagerImpl>(initInfo);
}

void PackageManagerImpl::Dispose()
{
  ClearAll();
  if (webSession != nullptr)
  {
    webSession->Dispose();
    webSession = nullptr;
  }
}

unique_ptr<PackageInstaller> PackageManagerImpl::CreateInstaller()
{
  return make_unique<PackageInstallerImpl>(shared_from_this());
}

unique_ptr<PackageIterator> PackageManagerImpl::CreateIterator()
{
  return make_unique<PackageIteratorImpl>(shared_from_this());
}

void PackageManagerImpl::LoadVariablePackageTable()
{
  // only load once
  if (commonVariablePackageTable != nullptr)
  {
    return;
  }

  commonVariablePackageTable = Cfg::Create();

  PathName pathCommonPackagesIni(pSession->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI, "");

  if (File::Exists(pathCommonPackagesIni))
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("loading common variable package table (%s)"), Q_(pathCommonPackagesIni));
    commonVariablePackageTable->Read(pathCommonPackagesIni);
  }

  commonVariablePackageTable->SetModified(false);

  PathName pathUserPackagesIni(pSession->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI, "");

  if (!pSession->IsAdminMode() && (pathCommonPackagesIni.Canonicalize() != pathUserPackagesIni.Canonicalize()))
  {
    userVariablePackageTable = Cfg::Create();
    if (File::Exists(pathUserPackagesIni))
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("loading user variable package table (%s)"), Q_(pathUserPackagesIni));
      userVariablePackageTable->Read(pathUserPackagesIni);
    }
    userVariablePackageTable->SetModified(false);
  }
}

void PackageManagerImpl::FlushVariablePackageTable()
{
  if (commonVariablePackageTable != nullptr
    && commonVariablePackageTable->IsModified())
  {
    PathName pathPackagesIni(pSession->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI, "");
    trace_mpm->WriteFormattedLine("libmpm", T_("flushing common variable package table (%s)"), Q_(pathPackagesIni));
    commonVariablePackageTable->Write(pathPackagesIni);
  }
  if (userVariablePackageTable != nullptr && userVariablePackageTable->IsModified())
  {
    PathName pathPackagesIni
      (pSession->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI, "");
    trace_mpm->WriteFormattedLine("libmpm", T_("flushing user variable package table (%s)"), Q_(pathPackagesIni));
    userVariablePackageTable->Write(pathPackagesIni);
  }
}

bool PackageManagerImpl::IsRemovable(const string & deploymentName)
{
  bool ret;
  LoadVariablePackageTable();
  string str;
  if (pSession->IsAdminMode())
  {
    // administrator can remove system-wide packages
    ret = GetCommonTimeInstalled(deploymentName) != 0;
  }
  else
  {
    // user can remove private packages
    if (pSession->GetSpecialPath(SpecialPath::CommonInstallRoot).Canonicalize() == pSession->GetSpecialPath(SpecialPath::UserInstallRoot).Canonicalize())
    {
      ret = GetTimeInstalled(deploymentName) != 0;
    }
    else
    {
      ret = GetUserTimeInstalled(deploymentName) != 0;
    }
  }
  return ret;
}

time_t PackageManagerImpl::GetUserTimeInstalled(const string & deploymentName)
{
  if (pSession->IsAdminMode())
  {
    MIKTEX_UNEXPECTED();
  }
  LoadVariablePackageTable();
  string str;
  if (userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(deploymentName, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

time_t PackageManagerImpl::GetCommonTimeInstalled(const string & deploymentName)
{
  LoadVariablePackageTable();
  string str;
  if (commonVariablePackageTable != nullptr && commonVariablePackageTable->TryGetValue(deploymentName, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

time_t PackageManagerImpl::GetTimeInstalled(const string & deploymentName)
{
  LoadVariablePackageTable();
  string str;
  if ((!pSession->IsAdminMode() && userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(deploymentName, "TimeInstalled", str))
    || commonVariablePackageTable->TryGetValue(deploymentName, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

bool PackageManagerImpl::IsPackageInstalled(const string & deploymentName)
{
  return GetTimeInstalled(deploymentName) > 0;
}

bool PackageManagerImpl::IsPackageObsolete(const string & deploymentName)
{
  LoadVariablePackageTable();
  string str;
  if ((!pSession->IsAdminMode()
    && userVariablePackageTable != nullptr
    && userVariablePackageTable->TryGetValue(deploymentName, "Obsolete", str))
    || commonVariablePackageTable->TryGetValue(deploymentName, "Obsolete", str))
  {
    return std::stoi(str) != 0;
  }
  else
  {
    return false;
  }
}

void PackageManagerImpl::DeclarePackageObsolete(const string & deploymentName, bool obsolete)
{
  LoadVariablePackageTable();
  if (pSession->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    commonVariablePackageTable->PutValue(deploymentName, "Obsolete", (obsolete ? "1" : "0"));
  }
  else
  {
    userVariablePackageTable->PutValue(deploymentName, "Obsolete", (obsolete ? "1" : "0"));
  }
}

void PackageManagerImpl::SetTimeInstalled(const string & deploymentName, time_t timeInstalled)
{
  LoadVariablePackageTable();
  if (pSession->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    if (timeInstalled == 0)
    {
      commonVariablePackageTable->DeleteKey(deploymentName);
    }
    else
    {
      commonVariablePackageTable->PutValue(deploymentName, "TimeInstalled", std::to_string(timeInstalled));
    }
  }
  else
  {
    if (timeInstalled == 0)
    {
      userVariablePackageTable->DeleteKey(deploymentName);
    }
    else
    {
      userVariablePackageTable->PutValue(deploymentName, "TimeInstalled", std::to_string(timeInstalled));
    }
  }
}

void PackageManagerImpl::SetReleaseState(const string & deploymentName, RepositoryReleaseState releaseState)
{
  LoadVariablePackageTable();
  if (pSession->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    commonVariablePackageTable->PutValue(deploymentName, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
  }
  else
  {
    userVariablePackageTable->PutValue(deploymentName, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
  }
}

RepositoryReleaseState PackageManagerImpl::GetReleaseState(const string & deploymentName)
{
  LoadVariablePackageTable();
  string str;
  if ((!pSession->IsAdminMode() && userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(deploymentName, "ReleaseState", str))
    || commonVariablePackageTable->TryGetValue(deploymentName, "ReleaseState", str))
  {
    if (str == "stable")
    {
      return RepositoryReleaseState::Stable;
    }
    else if (str == "next")
    {
      return RepositoryReleaseState::Next;
    }
  }
  return RepositoryReleaseState::Unknown;
}

void PackageManagerImpl::IncrementFileRefCounts(const vector<string> & files)
{
  for (vector<string>::const_iterator it = files.begin(); it != files.end(); ++it)
  {
    ++installedFileInfoTable[*it].refCount;
#if POLLUTE_THE_DEBUG_STREAM
    if (installedFileInfoTable[*it].refCount >= 2)
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: ref count > 1"), Q_(*it));
    }
#endif
  }
}

void PackageManagerImpl::IncrementFileRefCounts(const string & deploymentName)
{
  NeedInstalledFileInfoTable();
  const PackageInfo & pi = packageTable[deploymentName];
  IncrementFileRefCounts(pi.runFiles);
  IncrementFileRefCounts(pi.docFiles);
  IncrementFileRefCounts(pi.sourceFiles);
}

PackageInfo * PackageManagerImpl::DefinePackage(const string & deploymentName, const PackageInfo & packageInfo)
{
  pair<PackageDefinitionTable::iterator, bool> p = packageTable.insert(make_pair(deploymentName, packageInfo));
  p.first->second.deploymentName = deploymentName;
  if (pSession->IsMiKTeXDirect())
  {
    // installed from the start
    p.first->second.isRemovable = false;
    p.first->second.isObsolete = false;
    p.first->second.timeInstalled = packageInfo.timePackaged;
  }
  else
  {
    p.first->second.isRemovable = IsRemovable(deploymentName);
    p.first->second.isObsolete = IsPackageObsolete(deploymentName);
    p.first->second.timeInstalled = GetTimeInstalled(deploymentName);
    if (p.first->second.IsInstalled())
    {
      p.first->second.releaseState = GetReleaseState(deploymentName);
    }
  }
  return &(p.first->second);
}

void PackageManagerImpl::ParseAllPackageDefinitionFilesInDirectory(const PathName & directory)
{
  trace_mpm->WriteFormattedLine("libmpm", T_("searching %s for package definition files"), Q_(directory));

  if (!Directory::Exists(directory))
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("package definition directory (%s) does not exist"), Q_(directory));
    return;
  }

  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(directory, "*" MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);

  vector<future<PackageInfo>> futurePackageInfoTable;

  // parse package definition files
  if (((int)ASYNC_LAUNCH_POLICY & (int)launch::async) != 0)
  {
    const size_t maxPackageFiles = 4000;
    File::SetMaxOpen(maxPackageFiles);
  }
  unsigned count = 0;
  DirectoryEntry direntry;
  while (pLister->GetNext(direntry))
  {
    PathName name(direntry.name);

    // get deployment name
    char szDeploymentName[BufferSizes::MaxPackageName];
    name.GetFileNameWithoutExtension(szDeploymentName);

    // ignore redefinition
    if (packageTable.find(szDeploymentName) != packageTable.end())
    {
#if 0
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: ignoring redefinition"), szDeploymentName);
#endif
      continue;
    }

    // parse package definition file
    futurePackageInfoTable.push_back(async(ASYNC_LAUNCH_POLICY, [](const PathName & path)
    {
      TpmParser tpmParser;
      tpmParser.Parse(path);
      return tpmParser.GetPackageInfo();
    }, PathName(directory, name, "")));
  }
  pLister->Close();

  for (future<PackageInfo> & fpi : futurePackageInfoTable)
  {
    PackageInfo packageInfo = fpi.get();

#if IGNORE_OTHER_SYSTEMS
    string targetSystems = packageInfo.targetSystem;
    if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: ignoring %s package"), packageInfo.deploymentName.c_str(), targetSystems.c_str());
      continue;
    }
#endif

#if POLLUTE_THE_DEBUG_STREAM
    trace_mpm->WriteFormattedLine("libmpm", T_("  adding %s"), packageInfo.deploymentName.c_str());
#endif

    count += 1;

    // insert into database
    PackageInfo * pPi = DefinePackage(packageInfo.deploymentName, packageInfo);

    // increment file ref counts, if package is installed
    if (pPi->timeInstalled > 0)
    {
      IncrementFileRefCounts(pPi->runFiles);
      IncrementFileRefCounts(pPi->docFiles);
      IncrementFileRefCounts(pPi->sourceFiles);
    }
  }

  trace_mpm->WriteFormattedLine("libmpm", T_("found %u package definition files"), static_cast<unsigned>(count));

  // determine dependencies
  for (PackageDefinitionTable::iterator it = packageTable.begin(); it != packageTable.end(); ++it)
  {
    // FIXME
    time_t timeInstalledMin = static_cast<time_t>(0xffffffffffffffffULL);
    time_t timeInstalledMax = 0;
    for (vector<string>::const_iterator it2 = it->second.requiredPackages.begin(); it2 != it->second.requiredPackages.end(); ++it2)
    {
      PackageDefinitionTable::iterator it3 = packageTable.find(*it2);
      if (it3 == packageTable.end())
      {
	trace_mpm->WriteFormattedLine("libmpm", T_("dependancy problem: %s is required by %s"), it2->c_str(), it->second.deploymentName.c_str());
      }
      else
      {
	it3->second.requiredBy.push_back(it->second.deploymentName);
	if (it3->second.timeInstalled < timeInstalledMin)
	{
	  timeInstalledMin = it3->second.timeInstalled;
	}
	if (it3->second.timeInstalled > timeInstalledMax)
	{
	  timeInstalledMax = it3->second.timeInstalled;
	}
      }
    }
    if (timeInstalledMin > 0)
    {
      if (it->second.IsPureContainer() || (it->second.IsInstalled() && it->second.timeInstalled < timeInstalledMax))
      {
	it->second.timeInstalled = timeInstalledMax;
      }
    }
  }

  PackageDefinitionTable::iterator it2;

  // create "Obsolete" container
  PackageInfo piObsolete;
  piObsolete.deploymentName = "_miktex-obsolete";
  piObsolete.displayName = T_("Obsolete");
  piObsolete.title = T_("Obsolete packages");
  piObsolete.description = T_("Packages that were removed from the MiKTeX package repository.");
  for (it2 = packageTable.begin(); it2 != packageTable.end(); ++it2)
  {
    if (!it2->second.IsContained()
      && !it2->second.IsContainer()
      && IsPackageObsolete(it2->second.deploymentName))
    {
      piObsolete.requiredPackages.push_back(it2->second.deploymentName);
      it2->second.requiredBy.push_back(piObsolete.deploymentName);
    }
  }
  if (piObsolete.requiredPackages.size() > 0)
  {
    // insert "Obsolete" into the database
    DefinePackage(piObsolete.deploymentName, piObsolete);
  }

  // create "Other" container
  PackageInfo piOther;
  piOther.deploymentName = "_miktex-all-the-rest";
  piOther.displayName = T_("Uncategorized");
  piOther.title = T_("Uncategorized packages");
  for (it2 = packageTable.begin(); it2 != packageTable.end(); ++it2)
  {
    if (!it2->second.IsContained() && !it2->second.IsContainer())
    {
      piOther.requiredPackages.push_back(it2->second.deploymentName);
      it2->second.requiredBy.push_back(piOther.deploymentName);
    }
  }
  if (piOther.requiredPackages.size() > 0)
  {
    // insert "Other" into the database
    DefinePackage(piOther.deploymentName, piOther);
  }
}

void PackageManagerImpl::ParseAllPackageDefinitionFiles()
{
  if (parsedAllPackageDefinitionFiles)
  {
    // we do this once
    return;
  }
  PathName userInstallRoot = pSession->GetSpecialPath(SpecialPath::UserInstallRoot);
  PathName commonInstallRoot = pSession->GetSpecialPath(SpecialPath::CommonInstallRoot);
  if (!pSession->IsAdminMode())
  {
    ParseAllPackageDefinitionFilesInDirectory(PathName(userInstallRoot, MIKTEX_PATH_PACKAGE_DEFINITION_DIR, ""));
    if (userInstallRoot.Canonicalize() == commonInstallRoot.Canonicalize())
    {
      parsedAllPackageDefinitionFiles = true;
      return;
    }
  }
  ParseAllPackageDefinitionFilesInDirectory(PathName(commonInstallRoot, MIKTEX_PATH_PACKAGE_DEFINITION_DIR, ""));
  parsedAllPackageDefinitionFiles = true;
}

void PackageManagerImpl::LoadDatabase(const PathName & path)
{
  // get the full path name
  PathName absPath(path);
  absPath.MakeAbsolute();

  // check to see whether it is an archive file or a directory
  bool isDirectory = Directory::Exists(absPath);

  unique_ptr<TemporaryDirectory> tempDir;

  PathName pathPackageInfoDir;

  if (isDirectory)
  {
    pathPackageInfoDir = absPath;
  }
  else
  {
    // create temporary directory
    tempDir = TemporaryDirectory::Create();

    pathPackageInfoDir = tempDir->GetPathName();

    // unpack the package definition files
    unique_ptr<MiKTeX::Extractor::Extractor> pExtractor(MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE));
    pExtractor->Extract(absPath, pathPackageInfoDir);
  }

  // read package definition files
  ParseAllPackageDefinitionFilesInDirectory(pathPackageInfoDir);

  parsedAllPackageDefinitionFiles = true;
}

void PackageManagerImpl::ClearAll()
{
  packageTable.clear();
  installedFileInfoTable.clear();
  if (commonVariablePackageTable != nullptr)
  {
    commonVariablePackageTable = nullptr;
  }
  if (userVariablePackageTable != nullptr)
  {
    userVariablePackageTable = nullptr;
  }
  parsedAllPackageDefinitionFiles = false;
}

void PackageManagerImpl::UnloadDatabase()
{
  ClearAll();
}

PackageInfo * PackageManagerImpl::TryGetPackageInfo(const string & deploymentName)
{
  PackageDefinitionTable::iterator it = packageTable.find(deploymentName);
  if (it != packageTable.end())
  {
    return &it->second;
  }
  if (parsedAllPackageDefinitionFiles)
  {
    return 0;
  }
  PathName pathPackageDefinitionFile;
  bool havePackageInfoFile = false;
  if (!pSession->IsAdminMode())
  {
    pathPackageDefinitionFile = pSession->GetSpecialPath(SpecialPath::UserInstallRoot);
    pathPackageDefinitionFile /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
    pathPackageDefinitionFile /= deploymentName;
    pathPackageDefinitionFile.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
    havePackageInfoFile = File::Exists(pathPackageDefinitionFile);
  }
  if (!havePackageInfoFile)
  {
    pathPackageDefinitionFile = pSession->GetSpecialPath(SpecialPath::CommonInstallRoot);
    pathPackageDefinitionFile /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
    pathPackageDefinitionFile /= deploymentName;
    pathPackageDefinitionFile.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
    havePackageInfoFile = File::Exists(pathPackageDefinitionFile);
  }
  if (!havePackageInfoFile)
  {
    return 0;
  }
  TpmParser tpmParser;
  tpmParser.Parse(pathPackageDefinitionFile);
#if IGNORE_OTHER_SYSTEMS
  string targetSystems = tpmParser.GetPackageInfo().targetSystem;
  if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
  {
    return 0;
  }
#endif
  return DefinePackage(deploymentName, tpmParser.GetPackageInfo());
}

bool PackageManagerImpl::TryGetPackageInfo(const string & deploymentName, PackageInfo & packageInfo)
{
  PackageInfo * pPackageInfo = TryGetPackageInfo(deploymentName);
  if (pPackageInfo == nullptr)
  {
    return false;
  }
  else
  {
    packageInfo = *pPackageInfo;
    return true;
  }
}

PackageInfo PackageManagerImpl::GetPackageInfo(const string & deploymentName)
{
  const PackageInfo * pPackageInfo = TryGetPackageInfo(deploymentName);
  if (pPackageInfo == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return *pPackageInfo;
}

unsigned long PackageManagerImpl::GetFileRefCount(const PathName & path)
{
  NeedInstalledFileInfoTable();
  InstalledFileInfoTable::const_iterator it = installedFileInfoTable.find(path.Get());
  if (it == installedFileInfoTable.end())
  {
    return 0;
  }
  return it->second.refCount;
}

void PackageManagerImpl::NeedInstalledFileInfoTable()
{
  ParseAllPackageDefinitionFiles();
}

bool PackageManager::TryGetRemotePackageRepository(
  string & url, RepositoryReleaseState & repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  repositoryReleaseState = RepositoryReleaseState::Unknown;
  if (session->TryGetConfigValue(
    MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url))
  {
    string str;
    if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, str))
    {
      if (str == "stable")
      {
	repositoryReleaseState = RepositoryReleaseState::Stable;
      }
      else if (str == "next")
      {
	repositoryReleaseState = RepositoryReleaseState::Next;
      }
    }
    return true;
  }
  return Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, url)
    && (PackageManagerImpl::DetermineRepositoryType(url) == RepositoryType::Remote);
}

string PackageManager::GetRemotePackageRepository(RepositoryReleaseState & repositoryReleaseState)
{
  string url;
  if (!TryGetRemotePackageRepository(url, repositoryReleaseState))
  {
    MIKTEX_UNEXPECTED();
  }
  return url;
}

MPMSTATICFUNC(bool) IsUrl(const string & url)
{
  string::size_type pos = url.find("://");
  if (pos == string::npos)
  {
    return false;
  }
  string scheme = url.substr(0, pos);
  for (string::const_iterator it = scheme.begin(); it != scheme.end(); ++it)
  {
    if (!isalpha(*it, locale()))
    {
      return false;
    }
  }
  return true;
}

RepositoryType PackageManagerImpl::DetermineRepositoryType(const string & repository)
{
  if (IsUrl(repository))
  {
    return RepositoryType::Remote;
  }

  if (!Utils::IsAbsolutePath(repository.c_str()))
  {
    MIKTEX_UNEXPECTED();
  }

  if (PackageManager::IsLocalPackageRepository(repository))
  {
    return RepositoryType::Local;
  }

  if (Utils::IsMiKTeXDirectRoot(repository))
  {
    return RepositoryType::MiKTeXDirect;
  }

  PathName path(repository);
  path /= MIKTEX_PATH_PACKAGES_INI;
  if (File::Exists(path))
  {
    return RepositoryType::MiKTeXInstallation;
  }

  MIKTEX_FATAL_ERROR_2(T_("Not a package repository."), "repository", repository);
}

void PackageManager::SetRemotePackageRepository(const string & url, RepositoryReleaseState repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url.c_str());
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, repositoryReleaseState == RepositoryReleaseState::Stable ? "stable" : (repositoryReleaseState == RepositoryReleaseState::Next ? "next" : "unknown"));
}

bool PackageManager::TryGetLocalPackageRepository(PathName & path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageManagerImpl::DetermineRepositoryType(str) == RepositoryType::Local))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetLocalPackageRepository()
{
  PathName path;
  if (!TryGetLocalPackageRepository(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetLocalPackageRepository(const PathName & path)
{
  Session::Get()->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, path.Get());
}

bool PackageManager::TryGetMiKTeXDirectRoot(PathName & path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageManagerImpl::DetermineRepositoryType(str) == RepositoryType::MiKTeXDirect))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetMiKTeXDirectRoot()
{
  PathName path;
  if (!TryGetMiKTeXDirectRoot(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetMiKTeXDirectRoot(const PathName & path)
{
  shared_ptr<Session> session = Session::Get();
  session
    ->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, path.Get());
}

bool PackageManager::TryGetDefaultPackageRepository(RepositoryType & repositoryType, RepositoryReleaseState & repositoryReleaseState, string & urlOrPath)
{
  shared_ptr<Session> session = Session::Get();
  repositoryReleaseState = RepositoryReleaseState::Unknown;
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, str))
  {
    if (str == "remote")
    {
      urlOrPath = GetRemotePackageRepository(repositoryReleaseState);
      repositoryType = RepositoryType::Remote;
    }
    else if (str == "local")
    {
      urlOrPath = GetLocalPackageRepository().Get();
      repositoryType = RepositoryType::Local;
    }
    else if (str == "direct")
    {
      urlOrPath = GetMiKTeXDirectRoot().Get();
      repositoryType = RepositoryType::MiKTeXDirect;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, urlOrPath))
  {
    repositoryType = PackageManagerImpl::DetermineRepositoryType(urlOrPath);
    return true;
  }
  else
  {
    return false;
  }
}

RepositoryType PackageManager::GetDefaultPackageRepository(RepositoryReleaseState & repositoryReleaseState, string & urlOrPath)
{
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (!TryGetDefaultPackageRepository(repositoryType, repositoryReleaseState, urlOrPath))
  {
    MIKTEX_UNEXPECTED();
  }
  return repositoryType;
}

void PackageManager::SetDefaultPackageRepository(RepositoryType repositoryType, RepositoryReleaseState repositoryReleaseState, const string & urlOrPath)
{
  shared_ptr<Session> session = Session::Get();
  if (repositoryType == RepositoryType::Unknown)
  {
    repositoryType = PackageManagerImpl::DetermineRepositoryType(urlOrPath);
  }
  string repositoryTypeStr;
  switch (repositoryType)
  {
  case RepositoryType::MiKTeXDirect:
    repositoryTypeStr = "direct";
    SetMiKTeXDirectRoot(urlOrPath);
    break;
  case RepositoryType::Local:
    repositoryTypeStr = "local";
    SetLocalPackageRepository(urlOrPath);
    break;
  case RepositoryType::Remote:
    repositoryTypeStr = "remote";
    SetRemotePackageRepository(urlOrPath, repositoryReleaseState);
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, repositoryTypeStr.c_str());
}

template<class RepositoryInfo_> MPMSTATICFUNC(RepositoryInfo) MakeRepositoryInfo(const RepositoryInfo_ * pMwsRepositoryInfo)
{
  RepositoryInfo repositoryInfo;
  if (pMwsRepositoryInfo->Country != nullptr)
  {
    repositoryInfo.country = *pMwsRepositoryInfo->Country;
  }
  repositoryInfo.timeDate = pMwsRepositoryInfo->Date;
  repositoryInfo.delay = pMwsRepositoryInfo->Delay;
  if (pMwsRepositoryInfo->Description != nullptr)
  {
    repositoryInfo.description = *pMwsRepositoryInfo->Description;
  }
  switch (pMwsRepositoryInfo->Integrity)
  {
  case mtrep__RepositoryIntegrity__Corrupted:
    repositoryInfo.integrity = RepositoryIntegrity::Corrupted;
    break;
  case mtrep__RepositoryIntegrity__Intact:
    repositoryInfo.integrity = RepositoryIntegrity::Intact;
    break;
  case mtrep__RepositoryIntegrity__Unknown:
    repositoryInfo.integrity = RepositoryIntegrity::Unknown;
    break;
  }
  switch (pMwsRepositoryInfo->Level)
  {
  case mtrep__PackageLevel__Essential:
    repositoryInfo.packageLevel = PackageLevel::Essential;
    break;
  case mtrep__PackageLevel__Basic:
    repositoryInfo.packageLevel = PackageLevel::Basic;
    break;
  case mtrep__PackageLevel__Advanced:
    repositoryInfo.packageLevel = PackageLevel::Advanced;
    break;
  case mtrep__PackageLevel__Complete:
    repositoryInfo.packageLevel = PackageLevel::Complete;
    break;
  }
  switch (pMwsRepositoryInfo->Status)
  {
  case mtrep__RepositoryStatus__Online:
    repositoryInfo.status = RepositoryStatus::Online;
    break;
  case mtrep__RepositoryStatus__Offline:
    repositoryInfo.status = RepositoryStatus::Offline;
    break;
  case mtrep__RepositoryStatus__Unknown:
    repositoryInfo.status = RepositoryStatus::Unknown;
    break;
  }
  if (pMwsRepositoryInfo->Url != nullptr)
  {
    repositoryInfo.url = *pMwsRepositoryInfo->Url;
  }
  repositoryInfo.version = pMwsRepositoryInfo->Version;
  switch (pMwsRepositoryInfo->ReleaseState)
  {
  case mtrep6__RepositoryReleaseState__Stable:
    repositoryInfo.releaseState = RepositoryReleaseState::Stable;
    break;
  case mtrep6__RepositoryReleaseState__Next:
    repositoryInfo.releaseState = RepositoryReleaseState::Next;
    break;
  }
  return repositoryInfo;
}

void PackageManagerImpl::DownloadRepositoryList()
{
  repositories.clear();

  MyRepositorySoapProxy repositorySoapProxy;
  ProxySettings proxySettings;
  if (TryGetProxy(WEBSVCURL, proxySettings) && proxySettings.useProxy)
  {
    repositorySoapProxy.proxy_host = proxySettings.proxy.c_str();
    repositorySoapProxy.proxy_port = proxySettings.port;
    if (proxySettings.authenticationRequired)
    {
      repositorySoapProxy.proxy_userid = proxySettings.user.c_str();
      repositorySoapProxy.proxy_passwd = proxySettings.password.c_str();
    }
  }
  ClientInfo<mtrep6__ClientInfo> clientInfo;
  _mtrep6__GetRepositories4 arg;
  arg.clientInfo = &clientInfo;
  arg.onlyOnline = true;
  arg.noCorrupted = true;
  arg.maxDelay = -1;
  arg.excludeHidden = true;
  arg.httpOnly = false;
  switch (repositoryReleaseState)
  {
  case RepositoryReleaseState::Stable:
    arg.releaseState = mtrep6__RepositoryReleaseState__Stable;
    break;
  case RepositoryReleaseState::Next:
    arg.releaseState = mtrep6__RepositoryReleaseState__Next;
    break;
  default:
    break;
  }
  _mtrep6__GetRepositories4Response resp;
  if (repositorySoapProxy.GetRepositories4(&arg, resp) != SOAP_OK)
  {
    FATAL_SOAP_ERROR(&repositorySoapProxy);
  }
  for (vector<mtrep6__RepositoryInfo3*>::const_iterator it = resp.GetRepositories4Result->RepositoryInfo3.begin(); it != resp.GetRepositories4Result->RepositoryInfo3.end(); ++it)
  {
    repositories.push_back(MakeRepositoryInfo(*it));
  }
}

string PackageManagerImpl::PickRepositoryUrl()
{
  MyRepositorySoapProxy repositorySoapProxy;
  ProxySettings proxySettings;
  if (TryGetProxy(WEBSVCURL, proxySettings) && proxySettings.useProxy)
  {
    repositorySoapProxy.proxy_host = proxySettings.proxy.c_str();
    repositorySoapProxy.proxy_port = proxySettings.port;
    if (proxySettings.authenticationRequired)
    {
      repositorySoapProxy.proxy_userid = proxySettings.user.c_str();
      repositorySoapProxy.proxy_passwd = proxySettings.password.c_str();
    }
  }
  ClientInfo<mtrep6__ClientInfo> clientInfo;
  _mtrep6__PickRepository4 arg;
  arg.clientInfo = &clientInfo;
  arg.favorHttp = true;
  arg.nearest = true;
  switch (repositoryReleaseState)
  {
  case RepositoryReleaseState::Stable:
    arg.releaseState = mtrep6__RepositoryReleaseState__Stable;
    break;
  case RepositoryReleaseState::Next:
    arg.releaseState = mtrep6__RepositoryReleaseState__Next;
    break;
  default:
    break;
  }
  _mtrep6__PickRepository4Response resp;
  if (repositorySoapProxy.PickRepository4(&arg, resp) != SOAP_OK)
  {
    FATAL_SOAP_ERROR(&repositorySoapProxy);
  }
  if (resp.PickRepository4Result->Url == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return *resp.PickRepository4Result->Url;
}

void PackageManagerImpl::TraceError(const char * lpszFormat, ...)
{
  va_list marker;
  va_start(marker, lpszFormat);
  trace_error->VTrace("libmpm", lpszFormat, marker);
  va_end(marker);
}

namespace {

  typedef unordered_set<string, hash_path, equal_path> SubDirectoryTable;

  struct DirectoryInfo
  {
    SubDirectoryTable subDirectoryNames;
    string fileNames;
    string packageNames;
  };


  typedef unordered_map<string, DirectoryInfo, hash_path, equal_path> DirectoryInfoTable;

  // directory info table: written by
  // PackageManagerImpl::CreateMpmFndb(); read by
  // PackageManagerImpl::ReadDirectory()
  DirectoryInfoTable directoryInfoTable;

}

MPMSTATICFUNC(void) RememberFileNameInfo(const string & prefixedFileName, const string & packageName)
{
  shared_ptr<Session> session = Session::Get();

  string fileName;

  // ignore non-texmf files
  if (!PackageManager::StripTeXMFPrefix(prefixedFileName, fileName))
  {
    return;
  }

  PathNameParser pathtok(fileName.c_str());

  if (pathtok.GetCurrent() == nullptr)
  {
    return;
  }

  // initialize root path: "//MiKTeX/[MPM]"
  PathName path = session->GetMpmRootPath();
  //  path += CURRENT_DIRECTORY;

  // lpsz1: current path name component
  const char * lpsz1 = pathtok.GetCurrent();

  // lpszName: file name component
  const char * lpszName = lpsz1;

  for (const char * lpsz2 = ++pathtok; lpsz2 != nullptr; lpsz2 = ++pathtok)
  {
    directoryInfoTable[path.Get()].subDirectoryNames.insert(lpsz1);
    lpszName = lpsz2;
#if defined(MIKTEX_WINDOWS)
    // make sure the the rest of the path contains slashes (not
    // backslashes)
    path.AppendAltDirectoryDelimiter();
#else
    path.AppendDirectoryDelimiter();
#endif
    path /= lpsz1;
    lpsz1 = lpsz2;
  }

  DirectoryInfo & directoryInfo = directoryInfoTable[path.Get()];
  directoryInfo.fileNames += lpszName;
  directoryInfo.fileNames += '\0';
  directoryInfo.packageNames += packageName;
  directoryInfo.packageNames += '\0';
}

bool PackageManagerImpl::ReadDirectory(const char * lpszPath, char ** ppSubDirectoryNames, char ** ppFileNames, char ** ppFileNameInfos)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  // get the directory info for the given path
  DirectoryInfo & directoryInfo = directoryInfoTable[lpszPath];

  string subDirectoryNames;
  for (SubDirectoryTable::const_iterator it = directoryInfo.subDirectoryNames.begin(); it != directoryInfo.subDirectoryNames.end(); ++it)
  {
    subDirectoryNames += *it;
    subDirectoryNames += '\0';
  }
  subDirectoryNames += '\0';
  MIKTEX_ASSERT(ppSubDirectoryNames != nullptr);
  *ppSubDirectoryNames = static_cast<char *>(malloc(subDirectoryNames.length()));
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  subDirectoryNames._Copy_s(*ppSubDirectoryNames, subDirectoryNames.length(), subDirectoryNames.length());
#else
  subDirectoryNames.copy(*ppSubDirectoryNames, subDirectoryNames.length());
#endif
  MIKTEX_ASSERT(ppFileNames != nullptr);
  directoryInfo.fileNames += '\0';
  *ppFileNames =
    static_cast<char *>(malloc(directoryInfo.fileNames.length()));
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  directoryInfo.fileNames._Copy_s(*ppFileNames, directoryInfo.fileNames.length(), directoryInfo.fileNames.length());
#else
  directoryInfo.fileNames.copy(*ppFileNames, directoryInfo.fileNames.length());
#endif
  MIKTEX_ASSERT(ppFileNameInfos != nullptr);
  directoryInfo.packageNames += '\0';
  *ppFileNameInfos = static_cast<char *>(malloc(directoryInfo.packageNames.length()));
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  directoryInfo.packageNames._Copy_s(*ppFileNameInfos, directoryInfo.packageNames.length(), directoryInfo.packageNames.length());
#else
  directoryInfo.packageNames.copy(*ppFileNameInfos, directoryInfo.packageNames.length());
#endif
  return true;
}

bool PackageManagerImpl::OnProgress(unsigned level, const char * lpszDirectory)
{
  UNUSED_ALWAYS(level);
  UNUSED_ALWAYS(lpszDirectory);
  return true;
}

void PackageManagerImpl::CreateMpmFndb()
{
  ParseAllPackageDefinitionFiles();

  // collect the file names
  for (PackageDefinitionTable::const_iterator it = packageTable.begin(); it != packageTable.end(); ++it)
  {
    const PackageInfo & pi = it->second;
    vector<string>::const_iterator it2;
    for (it2 = pi.runFiles.begin(); it2 != pi.runFiles.end(); ++it2)
    {
      RememberFileNameInfo(*it2, pi.deploymentName);
    }
    for (it2 = pi.docFiles.begin(); it2 != pi.docFiles.end(); ++it2)
    {
      RememberFileNameInfo(*it2, pi.deploymentName);
    }
    for (it2 = pi.sourceFiles.begin(); it2 != pi.sourceFiles.end(); ++it2)
    {
      RememberFileNameInfo(*it2, pi.deploymentName);
    }
  }

  // create the database
  Fndb::Create(pSession->GetMpmDatabasePathName().Get(), pSession->GetMpmRootPath().Get(), this, true, true);

  // free memory
  directoryInfoTable.clear();
}

void PackageManagerImpl::GetAllPackageDefinitions(vector<PackageInfo> & packages)
{
  ParseAllPackageDefinitionFiles();
  for (PackageDefinitionTable::const_iterator it = packageTable.begin(); it != packageTable.end(); ++it)
  {
    packages.push_back(it->second);
  }
}

InstalledFileInfo * PackageManagerImpl::GetInstalledFileInfo(const char * lpszPath)
{
  ParseAllPackageDefinitionFiles();
  InstalledFileInfoTable::iterator it = installedFileInfoTable.find(lpszPath);
  if (it == installedFileInfoTable.end())
  {
    return nullptr;
  }
  return &it->second;
}

bool PackageManager::IsLocalPackageRepository(const PathName & path)
{
  if (!Directory::Exists(path))
  {
    return false;
  }

  // local mirror of remote package repository?
  if (File::Exists(PathName(path, MIKTEX_MPM_DB_LIGHT_FILE_NAME))
    && File::Exists(PathName(path, MIKTEX_MPM_DB_FULL_FILE_NAME)))
  {
    return true;
  }

  return false;
}

PackageInfo PackageManager::ReadPackageDefinitionFile(const PathName & path, const string & texmfPrefix)
{
  TpmParser tpmParser;
  tpmParser.Parse(path, texmfPrefix);
  return tpmParser.GetPackageInfo();
}

class XmlWriter
{
public:
  XmlWriter(FILE * stream)
    : stream(stream)
  {
    FPutS("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", stream);
  }

public:
  void StartElement(const char * lpszName)
  {
    if (freshElement)
    {
      FPutC('>', stream);
    }
    FPutC('<', stream);
    FPutS(lpszName, stream);
    freshElement = true;
    elements.push(lpszName);
  }

public:
  void AddAttribute(const char * lpszAttributeName, const char * lpszAttributeValue)
  {
    FPutC(' ', stream);
    FPutS(lpszAttributeName, stream);
    FPutS("=\"", stream);
    FPutS(lpszAttributeValue, stream);
    FPutC('"', stream);
  }

public:
  void EndElement()
  {
    if (elements.empty())
    {
      MIKTEX_UNEXPECTED();
    }
    if (freshElement)
    {
      FPutS("/>", stream);
      freshElement = false;
    }
    else
    {
      FPutS("</", stream);
      FPutS(elements.top().c_str(), stream);
      FPutC('>', stream);
    }
    elements.pop();
  }

public:
  void EndAllElements()
  {
    while (!elements.empty())
    {
      EndElement();
    }
  }

public:
  void Text(const string & text)
  {
    if (freshElement)
    {
      FPutC('>', stream);
      freshElement = false;
    }
    for (const char * lpszText = text.c_str(); *lpszText != 0; ++lpszText)
    {
      switch (*lpszText)
      {
      case '&':
	FPutS("&amp;", stream);
	break;
      case '<':
	FPutS("&lt;", stream);
	break;
      case '>':
	FPutS("&gt;", stream);
	break;
      default:
	FPutC(*lpszText, stream);
	break;
      }
    }
  }

private:
  FILE * stream;

private:
  stack<string> elements;

private:
  bool freshElement = false;
};

void PackageManager::WritePackageDefinitionFile(const PathName & path, const PackageInfo & packageInfo, time_t timePackaged)
{
  FileStream stream(File::Open(path, FileMode::Create, FileAccess::Write, false));

  XmlWriter xml(stream.Get());

  // create "rdf:Description" node
  xml.StartElement("rdf:RDF");
  xml.AddAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
  xml.AddAttribute("xmlns:TPM", "http://texlive.dante.de/");
  xml.StartElement("rdf:Description");
  string about("http://www.miktex.org/packages/");
  about += packageInfo.deploymentName;
  xml.AddAttribute("about", about.c_str());

  // create "TPM:Name" node
  xml.StartElement("TPM:Name");
  xml.Text(packageInfo.displayName);
  xml.EndElement();

  // create "TPM:Creator" node
  xml.StartElement("TPM:Creator");
  xml.Text("mpc");
  xml.EndElement();

  // create "TPM:Title" node
  xml.StartElement("TPM:Title");
  xml.Text(packageInfo.title);
  xml.EndElement();

  // create "TPM:Version" node
  xml.StartElement("TPM:Version");
  xml.Text(packageInfo.version);
  xml.EndElement();

  // create "TPM:TargetSystem" node
  xml.StartElement("TPM:TargetSystem");
  xml.Text(packageInfo.targetSystem);
  xml.EndElement();

  // create "TPM:Description" node
  xml.StartElement("TPM:Description");
  xml.Text(packageInfo.description);
  xml.EndElement();


  // create "TPM:RunFiles" node
  if (packageInfo.runFiles.size() > 0)
  {
    xml.StartElement("TPM:RunFiles");
    xml.AddAttribute("size", std::to_string(static_cast<unsigned>(packageInfo.sizeRunFiles)).c_str());
    for (vector<string>::const_iterator it = packageInfo.runFiles.begin(); it != packageInfo.runFiles.end(); ++it)
    {
      if (it != packageInfo.runFiles.begin())
      {
	xml.Text(" ");
      }
      xml.Text(*it);
    }
    xml.EndElement();
  }

  // create "TPM:DocFiles" node
  if (packageInfo.docFiles.size() > 0)
  {
    xml.StartElement("TPM:DocFiles");
    xml.AddAttribute("size", std::to_string(static_cast<unsigned>(packageInfo.sizeDocFiles)).c_str());
    for (vector<string>::const_iterator it = packageInfo.docFiles.begin(); it != packageInfo.docFiles.end(); ++it)
    {
      if (it != packageInfo.docFiles.begin())
      {
	xml.Text(" ");
      }
      xml.Text(*it);
    }
    xml.EndElement();
  }

  // create "TPM:SourceFiles" node
  if (packageInfo.sourceFiles.size())
  {
    xml.StartElement("TPM:SourceFiles");
    xml.AddAttribute("size", std::to_string(static_cast<unsigned>(packageInfo.sizeSourceFiles)).c_str());
    for (vector<string>::const_iterator it = packageInfo.sourceFiles.begin(); it != packageInfo.sourceFiles.end(); ++it)
    {
      if (it != packageInfo.sourceFiles.begin())
      {
	xml.Text(" ");
      }
      xml.Text(*it);
    }
    xml.EndElement();
  }

  // create "TPM:Requires" node
  if (packageInfo.requiredPackages.size() > 0)
  {
    xml.StartElement("TPM:Requires");
    for (vector<string>::const_iterator it = packageInfo.requiredPackages.begin(); it != packageInfo.requiredPackages.end(); ++it)
    {
      xml.StartElement("TPM:Package");
      xml.AddAttribute("name", it->c_str());
      xml.EndElement();
    }
    xml.EndElement();
  }

  // create "TPM:TimePackaged" node
  if (timePackaged != 0)
  {
    xml.StartElement("TPM:TimePackaged");
    xml.Text(std::to_string(timePackaged));
    xml.EndElement();
  }

  // create "TPM:MD5" node
  xml.StartElement("TPM:MD5");
  xml.Text(packageInfo.digest.ToString());
  xml.EndElement();

#if MIKTEX_EXTENDED_PACKAGEINFO
  if (!packageInfo.ctanPath.empty())
  {
    xml.StartElement("TPM:CTAN");
    xml.AddAttribute("path", packageInfo.ctanPath.c_str());
    xml.EndElement();
  }

  if (!(packageInfo.copyrightOwner.empty() && packageInfo.copyrightYear.empty()))
  {
    xml.StartElement("TPM:Copyright");
    xml.AddAttribute("owner", packageInfo.copyrightOwner.c_str());
    xml.AddAttribute("year", packageInfo.copyrightYear.c_str());
    xml.EndElement();
  }

  if (!packageInfo.licenseType.empty())
  {
    xml.StartElement("TPM:License");
    xml.AddAttribute("type", packageInfo.licenseType.c_str());
    xml.EndElement();
  }
#endif

  xml.EndAllElements();

  stream.Close();
}

bool PackageManager::StripTeXMFPrefix(const string & str, string & result)
{
  if (StripPrefix(str, TEXMF_PREFIX_DIRECTORY, result))
  {
    return true;
  }
  PathName prefix2(".");
  prefix2 /= TEXMF_PREFIX_DIRECTORY;
  return StripPrefix(str, prefix2.Get(), result);
}

void PackageManager::SetProxy(const ProxySettings & proxySettings)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, proxySettings.useProxy);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy.c_str());
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, proxySettings.port);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, proxySettings.authenticationRequired);
  PackageManagerImpl::proxyUser = proxySettings.user;
  PackageManagerImpl::proxyPassword = proxySettings.password;
}

bool PackageManager::TryGetProxy(const string & url, ProxySettings & proxySettings)
{
  shared_ptr<Session> pSession = Session::Get();
  string useProxy;
  if (pSession->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, useProxy))
  {
    proxySettings.useProxy = (useProxy == "t");
    if (!pSession->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy))
    {
      return false;
    }
    string port;
    if (!pSession->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, port))
    {
      return false;
    }
    proxySettings.port = std::stoi(port);
    string authenticationRequired;
    if (!pSession->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, authenticationRequired))
    {
      return false;
    }
    proxySettings.authenticationRequired = (authenticationRequired == "t");
    proxySettings.user = PackageManagerImpl::proxyUser;
    proxySettings.password = PackageManagerImpl::proxyPassword;
    return true;
  }
  string proxyEnv;
  if (!url.empty())
  {
    Uri uri(url.c_str());
    string scheme = uri.GetScheme();
    string envName;
    if (scheme == "http")
    {
      envName = "http_proxy";
    }
    else if (scheme == "ftp")
    {
      envName = "FTP_PROXY";
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
    Utils::GetEnvironmentString(envName.c_str(), proxyEnv);
  }
  if (proxyEnv.empty())
  {
    Utils::GetEnvironmentString("ALL_PROXY", proxyEnv);
  }
  if (proxyEnv.empty())
  {
    return false;
  }
  Uri uri(proxyEnv.c_str());
  proxySettings.useProxy = true;
  proxySettings.proxy = uri.GetHost();
  proxySettings.port = uri.GetPort();
  string userInfo = uri.GetUserInfo();
  proxySettings.authenticationRequired = !userInfo.empty();
  if (proxySettings.authenticationRequired)
  {
    string::size_type idx = userInfo.find_first_of(":");
    if (idx == string::npos)
    {
      proxySettings.user = userInfo;
      proxySettings.password = "";
    }
    else
    {
      proxySettings.user = userInfo.substr(0, idx);
      proxySettings.password = userInfo.substr(idx + 1);
    }
  }
  else
  {
    proxySettings.user = "";
    proxySettings.password = "";
  }
  return true;
}

bool PackageManager::TryGetProxy(ProxySettings & proxySettings)
{
  return TryGetProxy("", proxySettings);
}

ProxySettings PackageManager::GetProxy(const string & url)
{
  ProxySettings proxySettings;
  if (!TryGetProxy(url, proxySettings))
  {
    MIKTEX_FATAL_ERROR(T_("No proxy host is configured."));
  }
  return proxySettings;
}

ProxySettings PackageManager::GetProxy()
{
  return GetProxy("");
}

void PackageManagerImpl::OnProgress()
{
}

bool PackageManagerImpl::TryGetRepositoryInfo(const string & url, RepositoryInfo & repositoryInfo)
{
  RepositoryType repositoryType = PackageManagerImpl::DetermineRepositoryType(url);
  if (repositoryType == RepositoryType::Remote)
  {
    MyRepositorySoapProxy repositorySoapProxy;
    ProxySettings proxySettings;
    if (TryGetProxy(WEBSVCURL, proxySettings) && proxySettings.useProxy)
    {
      repositorySoapProxy.proxy_host = proxySettings.proxy.c_str();
      repositorySoapProxy.proxy_port = proxySettings.port;
      if (proxySettings.authenticationRequired)
      {
	repositorySoapProxy.proxy_userid = proxySettings.user.c_str();
	repositorySoapProxy.proxy_passwd = proxySettings.password.c_str();
      }
    }
    ClientInfo<mtrep7__ClientInfo> clientInfo;
    string url2 = url;
    _mtrep7__TryGetRepositoryInfo3 arg;
    arg.clientInfo = &clientInfo;
    arg.url = &url2;
    _mtrep7__TryGetRepositoryInfo3Response resp;
    if (repositorySoapProxy.TryGetRepositoryInfo3(&arg, resp) != SOAP_OK)
    {
      FATAL_SOAP_ERROR(&repositorySoapProxy);
    }
    if (resp.TryGetRepositoryInfo3Result)
    {
      repositoryInfo = MakeRepositoryInfo(resp.repositoryInfo);
    }
    return resp.TryGetRepositoryInfo3Result;
  }
  else if (repositoryType == RepositoryType::Local)
  {
    PathName configFile(url);
    configFile /= "pr.ini";
    unique_ptr<Cfg> pConfig(Cfg::Create());
    pConfig->Read(configFile);
    string value = pConfig->GetValue("repository", "date");
    repositoryInfo.timeDate = std::stoi(value);
    return true;
  }
  else
  {
    return false;
  }
}

RepositoryInfo PackageManagerImpl::VerifyPackageRepository(const string & url)
{
#if defined(_DEBUG)
  if (url == "http://ctan.miktex.org/systems/win32/miktex/tm/packages/")
  {
    RepositoryInfo repositoryInfo;
    repositoryInfo.delay = 0;
    return repositoryInfo;
  }
#endif
  for (vector<RepositoryInfo>::const_iterator it = repositories.begin(); it != repositories.end(); ++it)
  {
    if (it->url == url)
    {
      return *it;
    }
  }
  RepositoryInfo repositoryInfo;
  MyRepositorySoapProxy repositorySoapProxy;
  ProxySettings proxySettings;
  if (TryGetProxy(WEBSVCURL, proxySettings) && proxySettings.useProxy)
  {
    repositorySoapProxy.proxy_host = proxySettings.proxy.c_str();
    repositorySoapProxy.proxy_port = proxySettings.port;
    if (proxySettings.authenticationRequired)
    {
      repositorySoapProxy.proxy_userid = proxySettings.user.c_str();
      repositorySoapProxy.proxy_passwd = proxySettings.password.c_str();
    }
  }
  string url2 = url;
  ClientInfo<mtrep7__ClientInfo> clientInfo;
  _mtrep7__VerifyRepository2 arg;
  arg.clientInfo = &clientInfo;
  arg.url = &url2;
  _mtrep7__VerifyRepository2Response resp;
  if (repositorySoapProxy.VerifyRepository2(&arg, resp) != SOAP_OK)
  {
    FATAL_SOAP_ERROR(&repositorySoapProxy);
  }
  if (!resp.VerifyRepository2Result)
  {
    if (resp.repositoryInfo == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The remote package repository is not registered. You have to choose another repository."), "url", url);
    }
    else if ((int)resp.repositoryInfo->Status != (int)RepositoryStatus::Online)
    {
      MIKTEX_FATAL_ERROR_2(T_("The remote package repository is not online. You have to choose another repository."), "url", url);
    }
    else if ((int)resp.repositoryInfo->Integrity == (int)RepositoryIntegrity::Corrupted)
    {
      MIKTEX_FATAL_ERROR_2(T_("The remote package repository is corrupted. You have to choose another repository."), "url", url);
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("The remote package repository is outdated. You have to choose another repository."), "url", url);
    }
  }
  repositoryInfo = MakeRepositoryInfo(resp.repositoryInfo);
  repositories.push_back(repositoryInfo);
  return repositoryInfo;
}

bool PackageManagerImpl::TryGetFileDigest(const PathName & prefix, const string & fileName, bool & haveDigest, MD5 & digest)
{
  string unprefixed;
  if (!StripTeXMFPrefix(fileName, unprefixed))
  {
    return true;
  }
  PathName path = prefix;
  path /= unprefixed;
  if (!File::Exists(path))
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("package verification failed: file %s does not exist"), Q_(path));
    return false;
  }
  if (path.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
  {
    haveDigest = false;
  }
  else
  {
    digest = MD5::FromFile(path);
    haveDigest = true;
  }
  return true;
}

bool PackageManagerImpl::TryCollectFileDigests(const PathName & prefix, const vector<string> & files, FileDigestTable & fileDigests)
{
  for (const string & fileName : files)
  {
    bool haveDigest;
    MD5 digest;
    if (!TryGetFileDigest(prefix, fileName, haveDigest, digest))
    {
      return false;
    }
    if (haveDigest)
    {
      fileDigests[fileName] = digest;
    }
  }
  return true;
}

bool PackageManagerImpl::TryVerifyInstalledPackage(const string & deploymentName)
{
  PackageInfo packageInfo = GetPackageInfo(deploymentName);

  PathName prefix;

  if (!pSession->IsAdminMode() && GetUserTimeInstalled(deploymentName) != static_cast<time_t>(0))
  {
    prefix = pSession->GetSpecialPath(SpecialPath::UserInstallRoot);
  }

  if (prefix.Empty())
  {
    prefix = pSession->GetSpecialPath(SpecialPath::CommonInstallRoot);
  }

  FileDigestTable fileDigests;

  if (!TryCollectFileDigests(prefix, packageInfo.runFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.docFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.sourceFiles, fileDigests))
  {
    return false;
  }

  MD5Builder md5Builder;

  for (const pair<string, MD5> p : fileDigests)
  {
    PathName path(p.first);
    // we must dosify the path name for backward compatibility
    path.ToDos();
    md5Builder.Update(path.Get(), path.GetLength());
    md5Builder.Update(p.second.data(), p.second.size());
  }

  bool ok = md5Builder.Final() == packageInfo.digest;

  if (!ok)
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("package %s verification failed: some files have been modified"), Q_(deploymentName));
    trace_mpm->WriteFormattedLine("libmpm", T_("expected digest: %s"), packageInfo.digest.ToString().c_str());
    trace_mpm->WriteFormattedLine("libmpm", T_("computed digest: %s"), md5Builder.GetMD5().ToString().c_str());
  }

  return ok;
}

string PackageManagerImpl::GetContainerPath(const string & deploymentName, bool useDisplayNames)
{
  string path;
  PackageInfo packageInfo = GetPackageInfo(deploymentName);
  for (size_t idx = 0; idx < packageInfo.requiredBy.size(); ++idx)
  {
    PackageInfo packageInfo2 = GetPackageInfo(packageInfo.requiredBy[idx]);
    if (packageInfo2.IsPureContainer())
    {
      // RECUSION
      path = GetContainerPath(packageInfo.requiredBy[idx], useDisplayNames);
      path += PathName::DirectoryDelimiter;
      if (useDisplayNames)
      {
	path += packageInfo2.displayName;
      }
      else
      {
	path += packageInfo2.deploymentName;
      }
      break;
    }
  }
  return path;
}
