/* LinuxOsRelease.cpp: 

   Copyright (C) 2018 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Cfg.h"
#include "miktex/Core/tux/LinuxOsRelease.h"

using namespace MiKTeX::Core;
using namespace std;


string Unquote(const string& s)
{
  size_t len = s.length();
  if (len < 2 || s[0] != '"' || s[len - 1] != '"')
  {
    return s;
  }
  return s.substr(1, len - 1);
}

LinuxOsRelease ReadOsRelease()
{
  LinuxOsRelease result;
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read("/etc/os-release");
  for (const shared_ptr<Cfg::Key>& key : cfg->GetKeys())
  {
    for (const shared_ptr<Cfg::Value>& val : key->GetValues())
    {
      if (val->GetName() == "ID")
      {
        result.id = Unquote(val->GetValue());
      }
      else if (val->GetName() == "PRETTY_NAME")
      {
        result.pretty_name = Unquote(val->GetValue());
      }
      else if (val->GetName() == "VERSION_ID")
      {
        result.version_id = Unquote(val->GetValue());
      }
      else if (val->GetName() == "VERSION")
      {
        result.version = Unquote(val->GetValue());
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
  }
};

LazyLinuxOsRelease linuxOsRelease;

LinuxOsRelease LinuxOsRelease::Get()
{
  return linuxOsRelease;
}
