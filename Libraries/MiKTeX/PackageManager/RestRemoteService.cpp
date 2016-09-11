/* RestRemoteService.cpp

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

#include "RestRemoteService.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace nlohmann;
using namespace std;

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
    return RepositoryReleaseState::Next;
  case 1:
    return RepositoryReleaseState::Stable;
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

RepositoryInfo Deserialize(const json & j_rep)
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
  }
  return rep;
}

vector<RepositoryInfo> RestRemoteService::GetRepositories(RepositoryReleaseState repositoryReleaseState)
{
  string url = endpoint + "repositories?releaseState=" + ToString(repositoryReleaseState);
  unique_ptr<WebFile> webFile(webSession->OpenUrl(url.c_str()));
  char buf[1024];
  size_t n;
  stringstream response;
  while ((n = webFile->Read(buf, sizeof(buf))) > 0)
  {
    response.write(buf, n);
  }
  vector<RepositoryInfo> result;
  for (const json & j_rep : json::parse(response))
  {
    result.push_back(Deserialize(j_rep));
  }
  return result;
}

string RestRemoteService::PickRepositoryUrl(RepositoryReleaseState repositoryReleaseState)
{
  UNIMPLEMENTED();
}

pair<bool, RepositoryInfo> RestRemoteService::TryGetRepositoryInfo(const string & url)
{
  UNIMPLEMENTED();
}

RepositoryInfo RestRemoteService::Verify(const string & url)
{
  UNIMPLEMENTED();
}
