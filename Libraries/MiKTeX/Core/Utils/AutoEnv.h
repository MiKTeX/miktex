/* AutoEnv.h:

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

#pragma once

BEGIN_INTERNAL_NAMESPACE;

class AutoEnv
{
public:
  void Set(const std::string& valueName, const std::string& value)
  {
    this->valueName = valueName;
    MIKTEX_ASSERT(!this->haveOldValue);
    this->haveOldValue = MiKTeX::Core::Utils::GetEnvironmentString(valueName, this->oldValue);
    MiKTeX::Core::Utils::SetEnvironmentString(valueName, value);
  }
public:
  void Restore()
  {
    MIKTEX_ASSERT(!valueName.empty());
    if (haveOldValue)
    {
      MiKTeX::Core::Utils::SetEnvironmentString(valueName, oldValue);
    }
    else
    {
      MiKTeX::Core::Utils::RemoveEnvironmentString(valueName);
    }
    valueName = "";
    haveOldValue = false;
  }
public:
  ~AutoEnv()
  {
    try
    {
      if (!valueName.empty())
      {
        Restore();
      }
    }
    catch (const std::exception&)
    {
    }
  }
private:
  std::string valueName;
private:
  bool haveOldValue = false;
private:
  std::string oldValue;
};

END_INTERNAL_NAMESPACE;
