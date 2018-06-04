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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(DBD8ED6A4EAB41A49E187B7912407D4D)
#define DBD8ED6A4EAB41A49E187B7912407D4D

#include <memory>
#include <vector>

#include <miktex/Core/Session>

#include "miktex/PackageManager/RepositoryInfo.h"

BEGIN_INTERNAL_NAMESPACE;

class PackageRepositoryDataStore
{
private:
  std::vector<MiKTeX::Packages::RepositoryInfo> repositories;
private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

END_INTERNAL_NAMESPACE;

#endif
