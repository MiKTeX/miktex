/* PackageIteratorImpl.h:                               -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

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

#if !defined(AE44FFDD26FB4B12B48A737FB0AFD61B)
#define AE44FFDD26FB4B12B48A737FB0AFD61B

#include <memory>

#include "miktex/PackageManager/PackageIterator.h"

#include "PackageManagerImpl.h"

BEGIN_INTERNAL_NAMESPACE;

class PackageIteratorImpl :
  public MiKTeX::Packages::PackageIterator
{
public:
  MIKTEXTHISCALL ~PackageIteratorImpl() override;

public:
  void MIKTEXTHISCALL Dispose() override;

public:
  void MIKTEXTHISCALL AddFilter(PackageFilterSet filter) override
  {
    this->filter += filter;
  }

public:
  bool MIKTEXTHISCALL GetNext(MiKTeX::Packages::PackageInfo& packageInfo) override;

public:
  PackageIteratorImpl(std::shared_ptr<PackageManagerImpl> packageManager);

private:
  std::shared_ptr<PackageManagerImpl> packageManager;

private:
  std::vector<MiKTeX::Packages::PackageInfo> snapshot;

private:
  std::vector<MiKTeX::Packages::PackageInfo>::const_iterator iter;

private:
  std::string requiredBy;

private:
  PackageFilterSet filter;
};

END_INTERNAL_NAMESPACE;

#endif
