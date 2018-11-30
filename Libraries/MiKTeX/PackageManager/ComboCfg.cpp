/* ComboCfg.cpp

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

#include "ComboCfg.h"

using namespace std;

using namespace MiKTeX::Core;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

void ComboCfg::Load(const PathName& fileNameUser_, const PathName& fileNameCommon_)
{
  fileNameUser = fileNameUser_;
  fileNameCommon = fileNameCommon_;
  fileNameUser.Canonicalize();
  fileNameCommon.Canonicalize();
  cfgCommon = Cfg::Create();
  if (File::Exists(fileNameCommon))
  {
    cfgCommon->Read(fileNameCommon);
  }
  cfgCommon->SetModified(false);
  if (!session->IsAdminMode() && fileNameCommon != fileNameUser)
  {
    cfgUser = Cfg::Create();
    if (File::Exists(fileNameUser))
    {
      cfgUser->Read(fileNameUser);
    }
    cfgUser->SetModified(false);
  }
}

void ComboCfg::Clear()
{
  fileNameUser.Clear();
  fileNameCommon.Clear();
  cfgCommon = nullptr;
  cfgUser = nullptr;
}

void ComboCfg::Save()
{
  if (cfgCommon != nullptr && cfgCommon->IsModified())
  {
    cfgCommon->Write(fileNameCommon);
  }
  if (cfgUser != nullptr && cfgUser->IsModified())
  {
    cfgUser->Write(fileNameUser);
  }
}

bool ComboCfg::TryGetValueAsString(const string& keyName, const string& valueName, string& value)
{
  return !session->IsAdminMode() && cfgUser != nullptr && cfgUser->TryGetValueAsString(keyName, valueName, value)
    || cfgCommon->TryGetValueAsString(keyName, valueName, value);
}

bool ComboCfg::TryGetValueAsString(ComboCfg::Scope scope, const string& keyName, const string& valueName, string& value)
{
  switch (scope)
  {
  case Scope::User:
    return cfgUser != nullptr && cfgUser->TryGetValueAsString(keyName, valueName, value);
  case Scope::Common:
    return cfgCommon != nullptr && cfgCommon->TryGetValueAsString(keyName, valueName, value);
  }
}

void ComboCfg::PutValue(const string& keyName, const string& valueName, const string& value)
{
  if (session->IsAdminMode() || cfgUser == nullptr)
  {
    cfgCommon->PutValue(keyName, valueName, value);
  }
  else
  {
    cfgUser->PutValue(keyName, valueName, value);
  }
}

void ComboCfg::DeleteKey(const string& keyName)
{
  if (session->IsAdminMode() || cfgUser == nullptr)
  {
    cfgCommon->DeleteKey(keyName);
  }
  else
  {
    cfgUser->DeleteKey(keyName);
  }
}

bool ComboCfg::Loaded() const
{
  return cfgUser != nullptr || cfgCommon != nullptr;
}
