/* RestRemoteService.cpp

   Copyright (C) 2001-2020 Christian Schenk

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

#include <nlohmann/json.hpp>

#include <miktex/Core/ConfigNames>
#include <miktex/Core/Session>

#include <miktex/PackageManager/PackageManager>

#include "exceptions.h"
#include "internal.h"
#include "RestRemoteService.h"
#include "text.h"

using namespace std;
using namespace std::chrono;

using namespace nlohmann;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

inline string ToString(RepositoryReleaseState releaseState)
{
  switch (releaseState)
  {
  case RepositoryReleaseState::Next:
    return "Next";
  case RepositoryReleaseState::Stable:
    return "Stable";
  default:
    MIKTEX_UNEXPECTED();
  }
}

inline RepositoryReleaseState ToRepositoryReleaseState(int releaseState)
{
  switch (releaseState)
  {
  case 0:
    return RepositoryReleaseState::Stable;
  case 1:
    return RepositoryReleaseState::Next;
  default:
    MIKTEX_UNEXPECTED();
  }
}

inline RepositoryIntegrity ToRepositoryIntegrity(int integrity)
{
  switch (integrity)
  {
  case 0:
    return RepositoryIntegrity::Intact;
  case 1:
    return RepositoryIntegrity::Corrupted;
  case 2:
    return RepositoryIntegrity::Unknown;
  default:
    MIKTEX_UNEXPECTED();
  }
}

inline PackageLevel ToPackageLevel(int packageLevel)
{
  switch (packageLevel)
  {
  case 0:
    return PackageLevel::Essential;
  case 1:
    return PackageLevel::Basic;
  case 2:
    return PackageLevel::Advanced;
  case 3:
    return PackageLevel::Complete;
  default:
    MIKTEX_UNEXPECTED();
  }
}

inline RepositoryStatus ToRepositoryStatus(int repositoryStatus)
{
  switch (repositoryStatus)
  {
  case 0:
    return RepositoryStatus::Online;
  case 1:
    return RepositoryStatus::Offline;
  case 2:
    return RepositoryStatus::Unknown;
  default:
    MIKTEX_UNEXPECTED();
  }
}

RepositoryInfo Deserialize(const json& j_rep)
{
  RepositoryInfo rep;
  for (json::const_iterator it = j_rep.begin(); it != j_rep.end(); ++ it)
  {
    if (it.key() == "url")
    {
      rep.url = it.value().get<string>();
    }
    else if (it.key() == "country")
    {
      rep.country = it.value().get<string>();
    }
    else if (it.key() == "date")
    {
      rep.timeDate = static_cast<time_t>(it.value().get<int>());
    }
    else if (it.key() == "delay")
    {
      rep.delay = it.value().get<int>();
    }
    else if (it.key() == "relativeDelay")
    {
      rep.relativeDelay = it.value().get<int>();
    }
    else if (it.key() == "description")
    {
      rep.description = it.value().get<string>();
    }
    else if (it.key() == "integrity")
    {
      rep.integrity = ToRepositoryIntegrity(it.value().get<int>());
    }
    else if (it.key() == "level")
    {
      rep.packageLevel = ToPackageLevel(it.value().get<int>());
    }
    else if (it.key() == "status")
    {
      rep.status = ToRepositoryStatus(it.value().get<int>()); 
    }
    else if (it.key() == "version")
    {
      rep.version = it.value().get<int>();
    }
    else if (it.key() == "countryCode")
    {
      // TODO
    }
    else if (it.key() == "city")
    {
      rep.town = it.value().get<string>();
    }
    else if (it.key() == "latitude")
    {
      // TODO
    }
    else if (it.key() == "longitude")
    {
      // TODO
    }
    else if (it.key() == "attributes")
    {
      // TODO
    }
    else if (it.key() == "releaseState")
    {
      rep.releaseState = ToRepositoryReleaseState(it.value().get<int>());
    }
    else if (it.key() == "ranking")
    {
      rep.ranking = it.value().get<int>();
    }
  }
  return rep;
}

void RestRemoteService::Initialize()
{
  string configAuthToken;
  string configAuthTokenNotValidAfter;
  if (session->TryGetConfigValue(MIKTEX_CONFIG_SECTION_MPM, "AuthToken", configAuthToken) &&
      session->TryGetConfigValue(MIKTEX_CONFIG_SECTION_MPM, "AuthTokenNotValidAfter", configAuthTokenNotValidAfter))
  {
    token = configAuthToken;
    tokenNotValidAfter = system_clock::from_time_t(Utils::ToTimeT(configAuthTokenNotValidAfter));
    SetAuthHeader(token);
  }
}

vector<RepositoryInfo> RestRemoteService::GetRepositories(RepositoryReleaseState repositoryReleaseState)
{
  SayHello();
  unique_ptr<WebFile> webFile(webSession->OpenUrl(MakeUrl("repositories", { "releaseState=" + ToString(repositoryReleaseState) })));
  char buf[1024];
  size_t n;
  stringstream response;
  while ((n = webFile->Read(buf, sizeof(buf))) > 0)
  {
    response.write(buf, n);
  }
  vector<RepositoryInfo> result;
  for (const json& j_rep : json::parse(response))
  {
    result.push_back(Deserialize(j_rep));
  }
  return result;
}

string RestRemoteService::PickRepositoryUrl(RepositoryReleaseState repositoryReleaseState)
{
  SayHello();
  unique_ptr<WebFile> webFile(webSession->OpenUrl(MakeUrl("repositories", { "releaseState=" + ToString(repositoryReleaseState), "orderBy=ranking", "take=1" })));
  char buf[1024];
  size_t n;
  stringstream response;
  while ((n = webFile->Read(buf, sizeof(buf))) > 0)
  {
    response.write(buf, n);
  }
  for (const json& j_rep : json::parse(response))
  {
    return Deserialize(j_rep).url;
  }
  MIKTEX_UNEXPECTED();
}

pair<bool, RepositoryInfo> RestRemoteService::TryGetRepositoryInfo(const string& repositoryUrl)
{
  SayHello();
  unique_ptr<WebFile> webFile(webSession->OpenUrl(MakeUrl("repositories/" + MD5::FromChars(repositoryUrl).ToString(), { })));
  char buf[1024];
  size_t n;
  stringstream response;
  try
  {
    while ((n = webFile->Read(buf, sizeof(buf))) > 0)
    {
      response.write(buf, n);
    }
  }
  catch (const NotFoundException&)
  {
    return make_pair(false, RepositoryInfo());
  }
  json j_response = json::parse(response);
  return make_pair(true, Deserialize(j_response));
}

RepositoryInfo RestRemoteService::Verify(const string& repositoryUrl)
{
  pair<bool, RepositoryInfo> p = TryGetRepositoryInfo(repositoryUrl);
  if (!p.first)
  {
    MIKTEX_FATAL_ERROR_2(T_("The remote package repository is not registered. You have to choose another repository."), "url", repositoryUrl);
  }
  else if (p.second.status != RepositoryStatus::Online)
  {
    MIKTEX_FATAL_ERROR_2(T_("The remote package repository is not online. You have to choose another repository."), "url", repositoryUrl);
  }
  else if (p.second.integrity == RepositoryIntegrity::Corrupted)
  {
    MIKTEX_FATAL_ERROR_2(T_("The remote package repository is corrupted. You have to choose another repository."), "url", repositoryUrl);
  }
  else if (p.second.relativeDelay > 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("The remote package repository is outdated. You have to choose another repository."), "url", repositoryUrl);
  }
  return p.second;
}

void RestRemoteService::SayHello()
{
  if (!token.empty() && system_clock::now() < tokenNotValidAfter)
  {
    return;
  }
  unordered_map<string, string> form;
  MiKTeXUserInfo userinfo;
  if (session->TryGetMiKTeXUserInfo(userinfo) && userinfo.IsMember())
  {
    form["userid"] = userinfo.userid;
  }
  form["apikey"] = MPM_AGENT;
  form["apisecret"] = "abrakadabra";
  unique_ptr<WebFile> webFile(webSession->OpenUrl(MakeUrl("hello", {}), form));
  char buf[1024];
  size_t n;
  stringstream response;
  while ((n = webFile->Read(buf, sizeof(buf))) > 0)
  {
    response.write(buf, n);
  }
  json j_response = json::parse(response);
  for (json::const_iterator it = j_response.begin(); it != j_response.end(); ++it)
  {
    if (it.key() == "access_token")
    {
      token = it.value().get<string>();
    }
    else if (it.key() == "expires_in")
    {
      chrono::seconds expiresIn(it.value().get<int>());
      tokenNotValidAfter = system_clock::now() + expiresIn;
    }
  }
  if (token.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  if (tokenNotValidAfter < system_clock::now() + chrono::minutes(3))
  {
    // TODO: remove config values
  }
  else
  {
    session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, "AuthToken", ConfigValue(token));
    session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, "AuthTokenNotValidAfter", ConfigValue(static_cast<int>(system_clock::to_time_t(tokenNotValidAfter))));
  }
  SetAuthHeader(token);
}
