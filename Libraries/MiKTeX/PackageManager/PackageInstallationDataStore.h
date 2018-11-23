/* PackageInstallationDataStore.h:                      -*- C++ -*-

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

#if !defined(C4939A5A964940F38CA68698DDEC6718)
#define C4939A5A964940F38CA68698DDEC6718

#include <ctime>

#include <memory>
#include <string>

#include <miktex/PackageManager/PackageManager>

#include "ComboCfg.h"

BEGIN_INTERNAL_NAMESPACE;

class PackageInstallationDataStore
{
public:
  PackageInstallationDataStore();

public:
  void Load();

public:
  void Save();

public:
  void Clear();

public:
  void SetTimeInstalled(const std::string& packageId, std::time_t timeInstalled);

public:
  std::time_t GetUserTimeInstalled(const std::string& packageId);

public:
  std::time_t GetCommonTimeInstalled(const std::string& packageId);

public:
  std::time_t GetTimeInstalled(const std::string& packageId);

public:
  bool IsInstalled(const std::string& packageId);

public:
  bool IsRemovable(const std::string& packageId);

public:
  void DeclareObsolete(const std::string& packageId, bool obsolete);

public:
  bool IsObsolete(const std::string& packageId);

public:
  void SetReleaseState(const std::string& packageId, MiKTeX::Packages::RepositoryReleaseState releaseState);

public:
  MiKTeX::Packages::RepositoryReleaseState GetReleaseState(const std::string& packageId);

private:
  ComboCfg comboCfg;

private:
  bool loaded = false;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

END_INTERNAL_NAMESPACE;

#endif
