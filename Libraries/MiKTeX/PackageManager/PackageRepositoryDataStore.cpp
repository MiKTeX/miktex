/* PackageRepositoryDataStore.cpp

   Copyright (C) 2018 Christian Schenk

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
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <miktex/Core/Cfg>
#include <miktex/Core/Registry>

#include "miktex/PackageManager/PackageManager.h"

#include "internal.h"
#include "PackageRepositoryDataStore.h"
#include "RemoteService.h"
#include "WebFile.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

void PackageRepositoryDataStore::Download()
{
  ProxySettings proxySettings;
  if (!IsUrl(GetRemoteServiceBaseUrl()) || !PackageManager::TryGetProxy(GetRemoteServiceBaseUrl(), proxySettings))
  {
    proxySettings.useProxy = false;
  }
  unique_ptr<RemoteService> remoteService = RemoteService::Create(GetRemoteServiceBaseUrl(), proxySettings);
  repositories = remoteService->GetRepositories(repositoryReleaseState);
}

const char* DEFAULT_REMOTE_SERVICE = "https://api2.miktex.org/";

string PackageRepositoryDataStore::GetRemoteServiceBaseUrl()
{
  if (remoteServiceBaseUrl.empty())
  {
    remoteServiceBaseUrl = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_SERVICE, DEFAULT_REMOTE_SERVICE).GetString();
  }
  return remoteServiceBaseUrl;
}

string PackageRepositoryDataStore::PickRepositoryUrl()
{
  ProxySettings proxySettings;
  if (!IsUrl(GetRemoteServiceBaseUrl()) || !PackageManager::TryGetProxy(GetRemoteServiceBaseUrl(), proxySettings))
  {
    proxySettings.useProxy = false;
  }
  unique_ptr<RemoteService> remoteService = RemoteService::Create(GetRemoteServiceBaseUrl(), proxySettings);
  return remoteService->PickRepositoryUrl(repositoryReleaseState);
}

bool PackageRepositoryDataStore::TryGetRepositoryInfo(const string& url, RepositoryInfo& repositoryInfo)
{
  RepositoryType repositoryType = PackageRepositoryDataStore::DetermineRepositoryType(url);
  if (repositoryType == RepositoryType::Remote)
  {
    ProxySettings proxySettings;
    if (!IsUrl(GetRemoteServiceBaseUrl()) || !PackageManager::TryGetProxy(GetRemoteServiceBaseUrl(), proxySettings))
    {
      proxySettings.useProxy = false;
    }
    unique_ptr<RemoteService> remoteService = RemoteService::Create(GetRemoteServiceBaseUrl(), proxySettings);
    pair<bool, RepositoryInfo> result = remoteService->TryGetRepositoryInfo(url);
    if (result.first)
    {
      repositoryInfo = result.second;
    }
    return result.first;
  }
  else if (repositoryType == RepositoryType::Local)
  {
    PathName configFile(url);
    configFile /= "pr.ini";
    unique_ptr<Cfg> pConfig(Cfg::Create());
    pConfig->Read(configFile);
    string value = pConfig->GetValue("repository", "date")->GetValue();
    repositoryInfo.timeDate = std::stoi(value);
    return true;
  }
  else
  {
    return false;
  }
}

RepositoryType PackageRepositoryDataStore::DetermineRepositoryType(const string& repository)
{
  if (IsUrl(repository))
  {
    return RepositoryType::Remote;
  }

  if (!Utils::IsAbsolutePath(repository))
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

RepositoryInfo PackageRepositoryDataStore::VerifyPackageRepository(const string& url)
{
#if defined(_DEBUG)
  if (url == "http://ctan.miktex.org/systems/win32/miktex/tm/packages/")
  {
    RepositoryInfo repositoryInfo;
    repositoryInfo.delay = 0;
    return repositoryInfo;
  }
#endif
  for (const RepositoryInfo& repository : repositories)
  {
    if (repository.url == url)
    {
      return repository;
    }
  }
  ProxySettings proxySettings;
  if (!IsUrl(GetRemoteServiceBaseUrl()) || !PackageManager::TryGetProxy(GetRemoteServiceBaseUrl(), proxySettings))
  {
    proxySettings.useProxy = false;
  }
  unique_ptr<RemoteService> remoteService = RemoteService::Create(GetRemoteServiceBaseUrl(), proxySettings);
  RepositoryInfo repositoryInfo = remoteService->Verify(url);
  repositories.push_back(repositoryInfo);
  return repositoryInfo;
}

RepositoryInfo PackageRepositoryDataStore::CheckPackageRepository(const string& url)
{
  RepositoryInfo repositoryInfo;
  repositoryInfo.url = url;
  if (!TryGetRepositoryInfo(url, repositoryInfo))
  {
    return repositoryInfo;
  }
  string urlLargeFile = MakeUrl(url, "cm-super.tar.lzma");
  unique_ptr<WebFile> webFile;
  try
  {
    webFile = webSession->OpenUrl(urlLargeFile);
  }
  catch (const MiKTeXException&)
  {
    MIKTEX_ASSERT(webFile == nullptr);
  }
  if (webFile == nullptr)
  {
    repositoryInfo.status = RepositoryStatus::Offline;
    return repositoryInfo;
  }
  repositoryInfo.status = RepositoryStatus::Online;
  try
  {
    unsigned char buf[32 * 1024];
    size_t received = 0;
    size_t n;
    clock_t start = clock();
    clock_t maxTime = start + 5 * CLOCKS_PER_SEC;
    while ((n = webFile->Read(buf, sizeof(buf))) > 0 && clock() < maxTime)
    {
      received += n;
    }
    clock_t end = clock();
    if (start == end)
    {
      end = start + 1;
    }
    repositoryInfo.dataTransferRate = static_cast<double>(received) / (end - start) * CLOCKS_PER_SEC;
  }
  catch (const MiKTeXException&)
  {
  }
  repositoryInfo.lastCheckTime = time(nullptr);
  SaveVariableRepositoryData(repositoryInfo);
  return repositoryInfo;
}

void PackageRepositoryDataStore::SaveVariableRepositoryData(const RepositoryInfo& repositoryInfo)
{
  unique_ptr<Cfg> cfg = Cfg::Create();
  PathName cfgFile(session->GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_REPOSITORIES_INI);
  if (File::Exists(cfgFile))
  {
    cfg->Read(cfgFile);
  }
  cfg->PutValue(repositoryInfo.url, "LastCheckTime", std::to_string(repositoryInfo.lastCheckTime));
  cfg->PutValue(repositoryInfo.url, "LastVisitTime", std::to_string(repositoryInfo.lastVisitTime));
  cfg->PutValue(repositoryInfo.url, "DataTransferRate", std::to_string(repositoryInfo.dataTransferRate));
  cfg->Write(cfgFile);
}

