/* NoRemoteService.h:                                   -*- C++ -*-

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

#if !defined(A5D4990A62304F209D9688619234D364)
#define A5D4990A62304F209D9688619234D364

#include "RemoteService.h"

BEGIN_INTERNAL_NAMESPACE;

class NoRemoteService :
  public RemoteService
{
private:
  std::vector<RepositoryInfo> repositories;

public:
  NoRemoteService(const std::vector<std::string> & wellKnownBaseUrls)
  {
    for (const std::string & url : wellKnownBaseUrls)
    {
      RepositoryInfo rep;
      rep.url = url + "systems/win32/miktex/tm/packages/";
      rep.packageLevel = PackageLevel::Complete;
      rep.status = RepositoryStatus::Online;
      rep.releaseState = RepositoryReleaseState::Stable;
      repositories.push_back(rep);
      rep.url += "next/";
      rep.releaseState = RepositoryReleaseState::Next;
      repositories.push_back(rep);
    }
  }
  
public:
  std::vector<MiKTeX::Packages::RepositoryInfo> GetRepositories(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override
  {
    std::vector<RepositoryInfo> result;
    for (const RepositoryInfo & rep : repositories)
    {
      if (rep.releaseState == repositoryReleaseState)
      {
        result.push_back(rep);
      }
    }
    return result;
  }

public:
  std::string PickRepositoryUrl(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override
  {
    for (const RepositoryInfo & rep : repositories)
    {
      if (rep.releaseState == repositoryReleaseState)
      {
        return rep.url;
      }
    }
    MIKTEX_FATAL_ERROR(T_("No package repository available."));
  }
  
public:
  std::pair<bool, MiKTeX::Packages::RepositoryInfo> TryGetRepositoryInfo(const std::string & repositoryUrl) override
  {
    for (const RepositoryInfo & rep : repositories)
    {
      if (rep.url == repositoryUrl)
      {
        return std::make_pair(true, rep);
      }
    }
    return std::make_pair(false, RepositoryInfo());
  }

public:
  MiKTeX::Packages::RepositoryInfo Verify(const std::string & repositoryUrl) override
  {
    std::pair<bool, RepositoryInfo> p = TryGetRepositoryInfo(repositoryUrl);
    if (!p.first)
    {
      MIKTEX_FATAL_ERROR_2(T_("The remote package repository is not registered. You have to choose another repository."), "url", repositoryUrl);
    }
    return p.second;
  }
};

END_INTERNAL_NAMESPACE;

#endif
