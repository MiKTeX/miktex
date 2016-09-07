/* SoapRemoteService.cpp

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

#include "SoapRemoteService.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

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

#define FATAL_SOAP_ERROR(pSoap) FatalSoapError(pSoap, MIKTEX_SOURCE_LOCATION())

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

vector<RepositoryInfo> SoapRemoteService::GetRepositories(RepositoryReleaseState repositoryReleaseState)
{
  vector<RepositoryInfo> repositories;
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
  for (const mtrep6__RepositoryInfo3 * r : resp.GetRepositories4Result->RepositoryInfo3)
  {
    repositories.push_back(MakeRepositoryInfo(r));
  }
  return repositories;
}

string SoapRemoteService::PickRepositoryUrl(RepositoryReleaseState repositoryReleaseState)
{
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

pair<bool, RepositoryInfo> SoapRemoteService::TryGetRepositoryInfo(const string & url)
{
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
  RepositoryInfo repositoryInfo;
  if (resp.TryGetRepositoryInfo3Result)
  {
    repositoryInfo = MakeRepositoryInfo(resp.repositoryInfo);
  }
  return make_pair(resp.TryGetRepositoryInfo3Result, repositoryInfo);
}

RepositoryInfo SoapRemoteService::Verify(const string & url)
{
  RepositoryInfo repositoryInfo;
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
  return repositoryInfo;
}
