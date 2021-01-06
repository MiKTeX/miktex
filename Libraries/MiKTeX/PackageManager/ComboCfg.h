/* ComboCfg.h:                                          -*- C++ -*-

   Copyright (C) 2018-2019 Christian Schenk

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

#if !defined(B1A7DA448CE140D291E771FF638D09CE)
#define B1A7DA448CE140D291E771FF638D09CE

#include <string>

#include <miktex/Core/Cfg>
#include <miktex/Util/PathName>
#include <miktex/Core/Session>

MPM_INTERNAL_BEGIN_NAMESPACE;

class ComboCfg
{
public:
  void Load(const MiKTeX::Util::PathName& fileNameUser, const MiKTeX::Util::PathName& fileNameCommon);

public:
  void Clear();

public:
  void Save();

public:
  bool TryGetValueAsString(const std::string& keyName, const std::string& valueName, std::string& value);

public:
  bool TryGetValueAsString(MiKTeX::Core::ConfigurationScope scope, const std::string& keyName, const std::string& valueName, std::string& value);

public:
  void PutValue(const std::string& keyName, const std::string& valueName, const std::string& value);

public:
  void DeleteKey(const std::string& keyName);

public:
  bool Loaded() const;

public:
  std::size_t GetSize(MiKTeX::Core::ConfigurationScope scope) const;

private:
  MiKTeX::Util::PathName fileNameUser;

private:
  MiKTeX::Util::PathName fileNameCommon;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> cfgUser;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> cfgCommon;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

MPM_INTERNAL_END_NAMESPACE;

#endif
