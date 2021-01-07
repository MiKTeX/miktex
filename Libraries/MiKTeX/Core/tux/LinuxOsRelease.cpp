/* LinuxOsRelease.cpp: 

   Copyright (C) 2018-2021 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <iomanip>

#include <miktex/Core/Cfg>
#include <miktex/Util/PathName>
#include <miktex/Core/tux/LinuxOsRelease>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

string Unquote(const string& s)
{
  string result;
  istringstream str(s);
  str >> std::quoted(result);
  return result;
}

LinuxOsRelease ReadOsRelease()
{
  LinuxOsRelease result;
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(PathName("/etc/os-release"));
  for (const shared_ptr<Cfg::Key>& key : *cfg)
  {
    for (const shared_ptr<Cfg::Value>& val : *key)
    {
      if (val->GetName() == "NAME")
      {
        result.name = Unquote(val->AsString());
      }
      else if (val->GetName() == "VERSION")
      {
        result.version = Unquote(val->AsString());
      }
      else if (val->GetName() == "ID")
      {
        result.id = Unquote(val->AsString());
      }
      else if (val->GetName() == "ID_LIKE")
      {
        result.id_like = Unquote(val->AsString());
      }
      else if (val->GetName() == "VERSION_CODENAME")
      {
        result.version_codename = Unquote(val->AsString());
      }
      else if (val->GetName() == "VERSION_ID")
      {
        result.version_id = Unquote(val->AsString());
      }
      else if (val->GetName() == "PRETTY_NAME")
      {
        result.pretty_name = Unquote(val->AsString());
      }
      else if (val->GetName() == "ANSI_COLOR")
      {
        result.ansi_color = Unquote(val->AsString());
      }
      else if (val->GetName() == "CPE_NAME")
      {
        result.cpe_name = Unquote(val->AsString());
      }
      else if (val->GetName() == "HOME_URL")
      {
        result.home_url = Unquote(val->AsString());
      }
      else if (val->GetName() == "SUPPORT_URL")
      {
        result.support_url = Unquote(val->AsString());
      }
      else if (val->GetName() == "BUG_REPORT_URL")
      {
        result.bug_report_url = Unquote(val->AsString());
      }
      else if (val->GetName() == "PRIVACY_POLICY_URL")
      {
        result.privacy_policy_url = Unquote(val->AsString());
      }
      else if (val->GetName() == "BUILD_ID")
      {
        result.build_id = Unquote(val->AsString());
      }
      else if (val->GetName() == "VARIANT")
      {
        result.variant = Unquote(val->AsString());
      }
      else if (val->GetName() == "VARIANT_ID")
      {
        result.variant_id = Unquote(val->AsString());
      }
    }
  }
  return result;
}

class LazyLinuxOsRelease
{
private:
  LinuxOsRelease linuxOsRelease;
public:
  operator LinuxOsRelease()
  {
    if (linuxOsRelease.id.empty())
    {
      linuxOsRelease = ReadOsRelease();
    }
    return linuxOsRelease;
  }
};

LinuxOsRelease LinuxOsRelease::Get()
{
  static LazyLinuxOsRelease linuxOsRelease;
  return linuxOsRelease;
}
