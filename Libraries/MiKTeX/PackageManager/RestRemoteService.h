/* RestRemoteService.h:                                 -*- C++ -*-

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
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(A965AF9537944AFEA8EE4AB0D04F55B5)
#define A965AF9537944AFEA8EE4AB0D04F55B5

#include "RemoteService.h"

BEGIN_INTERNAL_NAMESPACE;

class RestRemoteService :
  public RemoteService
{
public:
  RestRemoteService(const std::string & endpoint, const MiKTeX::Packages::ProxySettings & proxySettings) :
    endpointBaseUrl(endpoint),
    proxySettings(proxySettings),
    webSession(WebSession::Create(nullptr))
  {
  }
  
public:
  std::vector<MiKTeX::Packages::RepositoryInfo> GetRepositories(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override;

public:
  std::string PickRepositoryUrl(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override;

public:
  std::pair<bool, MiKTeX::Packages::RepositoryInfo> TryGetRepositoryInfo(const std::string & repositoryUrl) override;

public:
  MiKTeX::Packages::RepositoryInfo Verify(const std::string & url) override;

private:
  std::string MakeUrl(const std::string & path, const std::initializer_list<std::string> & query)
  {
    std::string url = endpointBaseUrl + path;
    if (query.size() > 0)
    {
      url += "?";
      for (const std::string & q : query)
      {
        url += "&" + q;
      }
    }
    return url;
  }

private:
  std::string endpointBaseUrl;

private:
  MiKTeX::Packages::ProxySettings proxySettings;

private:
  std::shared_ptr<WebSession> webSession;
};

END_INTERNAL_NAMESPACE;

#endif
