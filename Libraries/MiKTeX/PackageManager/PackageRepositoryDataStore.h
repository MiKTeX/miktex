/* PackageRepositoryDataStore.h:                        -*- C++ -*-

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

#pragma once

#if !defined(DBD8ED6A4EAB41A49E187B7912407D4D)
#define DBD8ED6A4EAB41A49E187B7912407D4D

#include <memory>
#include <string>
#include <vector>

#include <miktex/Core/Session>

#include <miktex/PackageManager/PackageManager>

#include "ComboCfg.h"
#include "WebSession.h"

BEGIN_INTERNAL_NAMESPACE;

class PackageRepositoryDataStore
{
public:
  PackageRepositoryDataStore(std::shared_ptr<WebSession> webSession);

public:
  void Download();

public:
  std::vector<MiKTeX::Packages::RepositoryInfo> GetRepositories() const
  {
    return repositories;
  }

public:
  std::string PickRepositoryUrl();

public:
  MiKTeX::Packages::RepositoryInfo CheckPackageRepository(const std::string& url);

public:
  bool TryGetRepositoryInfo(const std::string& url, RepositoryInfo& repositoryInfo);

public:
  MiKTeX::Packages::RepositoryInfo VerifyPackageRepository(const std::string& url);

public:
  static MiKTeX::Packages::RepositoryType DetermineRepositoryType(const std::string& repository);

public:
  MiKTeX::Packages::RepositoryReleaseState GetRepositoryReleaseState()
  {
    return repositoryReleaseState;
  }

public:
  void SetRepositoryReleaseState(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState)
  {
    this->repositoryReleaseState = repositoryReleaseState;
  }

private:
  void LoadVarData(MiKTeX::Packages::RepositoryInfo& repositoryInfo);

private:
  void SaveVarData(const MiKTeX::Packages::RepositoryInfo& repositoryInfo);

private:
  std::string GetRemoteServiceBaseUrl();

private:
  std::vector<MiKTeX::Packages::RepositoryInfo> repositories;

private:
#if MIKTEX_RELEASE_STATE == 1
  MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Next;
#else
  MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Stable;
#endif

private:
  std::string remoteServiceBaseUrl;

private:
  std::shared_ptr<WebSession> webSession;

private:
  ComboCfg comboCfg;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

END_INTERNAL_NAMESPACE;

#endif
