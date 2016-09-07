/* SoapRemoteService.h:                                 -*- C++ -*-

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

#if !defined(D7BDFD4C0DE446F4BFE09214304D0CBD)
#define D7BDFD4C0DE446F4BFE09214304D0CBD

#include "../WebServiceClients/Repository/repositoryRepositorySoapProxy.h"

#include "RemoteService.h"

BEGIN_INTERNAL_NAMESPACE;

class SoapRemoteService :
  public RemoteService
{
public:
  SoapRemoteService(const std::string & endpoint, const MiKTeX::Packages::ProxySettings & proxySettings) :
    endpoint(endpoint),
    proxySettings(proxySettings)
  {
    repositorySoapProxy.soap_endpoint = this->endpoint.c_str();
    if (this->proxySettings.useProxy)
    {
      repositorySoapProxy.proxy_host = this->proxySettings.proxy.c_str();
      repositorySoapProxy.proxy_port = this->proxySettings.port;
      if (this->proxySettings.authenticationRequired)
      {
	repositorySoapProxy.proxy_userid = this->proxySettings.user.c_str();
	repositorySoapProxy.proxy_passwd = this->proxySettings.password.c_str();
      }
    }
  }
  
public:
  std::vector<MiKTeX::Packages::RepositoryInfo> GetRepositories(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override;

public:
  std::string PickRepositoryUrl(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override;

public:
  std::pair<bool, MiKTeX::Packages::RepositoryInfo> TryGetRepositoryInfo(const std::string & url) override;

public:
  MiKTeX::Packages::RepositoryInfo Verify(const std::string & url) override;

private:
  RepositorySoapProxy repositorySoapProxy;

private:
  std::string endpoint;

private:
  MiKTeX::Packages::ProxySettings proxySettings;
};

END_INTERNAL_NAMESPACE;

#endif
