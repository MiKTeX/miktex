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
#include <miktex/Core/Uri>

#include "miktex/PackageManager/PackageManager.h"

#include "internal.h"
#include "PackageRepositoryDataStore.h"
#include "RemoteService.h"
#include "WebFile.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

PackageRepositoryDataStore::PackageRepositoryDataStore(std::shared_ptr<WebSession> webSession) :
  webSession(webSession)
{
  MIKTEX_ASSERT(webSession != nullptr);
  comboCfg.Load(
    session->GetSpecialPath(SpecialPath::UserConfigRoot) / MIKTEX_PATH_REPOSITORIES_INI,
    session->GetSpecialPath(SpecialPath::CommonConfigRoot) / MIKTEX_PATH_REPOSITORIES_INI);
}

void PackageRepositoryDataStore::Download()
{
  ProxySettings proxySettings;
  if (!IsUrl(GetRemoteServiceBaseUrl()) || !PackageManager::TryGetProxy(GetRemoteServiceBaseUrl(), proxySettings))
  {
    proxySettings.useProxy = false;
  }
  unique_ptr<RemoteService> remoteService = RemoteService::Create(GetRemoteServiceBaseUrl(), proxySettings);
  repositories = remoteService->GetRepositories(repositoryReleaseState);
  for (RepositoryInfo& r : repositories)
  {
    LoadVarData(r);
  }
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
    unique_ptr<Cfg> config(Cfg::Create());
    config->Read(configFile);
    auto value = config->GetValue("repository", "date");
    if (value == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    repositoryInfo.timeDate = Utils::ToTimeT(value->AsString());
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
  SaveVarData(repositoryInfo);
  return repositoryInfo;
}

string MakeKey(const string& url)
{
  Uri uri(url);
  return uri.GetScheme() + "://" + uri.GetHost();
}

void PackageRepositoryDataStore::LoadVarData(RepositoryInfo& repositoryInfo)
{
  string key = MakeKey(repositoryInfo.url);
  string val;
  if (comboCfg.TryGetValueAsString(key, "LastCheckTime", val))
  {
    repositoryInfo.lastCheckTime = Utils::ToTimeT(val);
  }
  if (comboCfg.TryGetValueAsString(key, "LastVisitTime", val))
  {
    repositoryInfo.lastVisitTime = Utils::ToTimeT(val);
  }
  if (comboCfg.TryGetValueAsString(key, "DataTransferRate", val))
  {
    repositoryInfo.dataTransferRate = std::stod(val);
  }
}

void PackageRepositoryDataStore::SaveVarData(const RepositoryInfo& repositoryInfo)
{
  string key = MakeKey(repositoryInfo.url);
  comboCfg.PutValue(key, "LastCheckTime", std::to_string(repositoryInfo.lastCheckTime));
  comboCfg.PutValue(key, "LastVisitTime", std::to_string(repositoryInfo.lastVisitTime));
  comboCfg.PutValue(key, "DataTransferRate", std::to_string(repositoryInfo.dataTransferRate));
  comboCfg.Save();
}
