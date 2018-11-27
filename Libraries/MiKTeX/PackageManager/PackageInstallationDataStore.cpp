/* PackageInstallationDataStore.cpp

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

#include "internal.h"

#include "PackageInstallationDataStore.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

PackageInstallationDataStore::PackageInstallationDataStore()
{
}

void PackageInstallationDataStore::Load()
{
  comboCfg.Load(
    session->GetSpecialPath(SpecialPath::UserInstallRoot) / MIKTEX_PATH_PACKAGES_INI,
    session->GetSpecialPath(SpecialPath::CommonInstallRoot) / MIKTEX_PATH_PACKAGES_INI);
  loaded = true;
}

void PackageInstallationDataStore::Save()
{
  comboCfg.Save();
}

void PackageInstallationDataStore::Clear()
{
  comboCfg.Clear();
  loaded = false;
}

void PackageInstallationDataStore::SetTimeInstalled(const string& packageId, time_t timeInstalled)
{
  if (!loaded)
  {
    Load();
  }
  if (IsValidTimeT(timeInstalled))
  {
    comboCfg.PutValue(packageId, "TimeInstalled", std::to_string(timeInstalled));
  }
  else
  {
    comboCfg.DeleteKey(packageId);
  }
}

time_t PackageInstallationDataStore::GetUserTimeInstalled(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  string str;
  if (comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

time_t PackageInstallationDataStore::GetCommonTimeInstalled(const std::string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  string str;
  if (comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

time_t PackageInstallationDataStore::GetTimeInstalled(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  string str;
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "TimeInstalled", str))
    || comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

bool PackageInstallationDataStore::IsInstalled(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  return IsValidTimeT(GetTimeInstalled(packageId));
}

bool PackageInstallationDataStore::IsRemovable(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  bool ret;
  string str;
  if (session->IsAdminMode())
  {
    // administrator can remove system-wide packages
    ret = IsValidTimeT(GetCommonTimeInstalled(packageId));
  }
  else
  {
    // user can remove private packages
    if (session->GetSpecialPath(SpecialPath::CommonInstallRoot).Canonicalize() == session->GetSpecialPath(SpecialPath::UserInstallRoot).Canonicalize())
    {
      ret = IsValidTimeT(GetTimeInstalled(packageId));
    }
    else
    {
      ret = IsValidTimeT(GetUserTimeInstalled(packageId));
    }
  }
  return ret;
}

void PackageInstallationDataStore::DeclareObsolete(const string& packageId, bool obsolete)
{
  if (!loaded)
  {
    Load();
  }
  comboCfg.PutValue(packageId, "Obsolete", (obsolete ? "1" : "0"));
}

bool PackageInstallationDataStore::IsObsolete(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  string str;
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "Obsolete", str))
    || comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "Obsolete", str))
  {
    return std::stoi(str) != 0;
  }
  else
  {
    return false;
  }
}

void PackageInstallationDataStore::SetReleaseState(const string& packageId, RepositoryReleaseState releaseState)
{
  if (!loaded)
  {
    Load();
  }
  comboCfg.PutValue(packageId, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
}

RepositoryReleaseState PackageInstallationDataStore::GetReleaseState(const string& packageId)
{
  if (!loaded)
  {
    Load();
  }
  string str;
  if (comboCfg.TryGetValueAsString(packageId, "ReleaseState", str))
  {
    if (str == "stable")
    {
      return RepositoryReleaseState::Stable;
    }
    else if (str == "next")
    {
      return RepositoryReleaseState::Next;
    }
  }
  return RepositoryReleaseState::Unknown;
}
