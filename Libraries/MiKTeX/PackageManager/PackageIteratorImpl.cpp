/* PackageIterator.cpp:

   Copyright (C) 2001-2020 Christian Schenk

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
   along with MiKTeX Package Manager; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "config.h"

#include "PackageIteratorImpl.h"

using namespace std;

using namespace MiKTeX::Packages;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

PackageIteratorImpl::PackageIteratorImpl(shared_ptr<PackageManagerImpl> packageManager, bool noLock) :
  packageManager(packageManager)
{
  if (noLock)
  {
    Init();
  }
  else
  {
    MPM_LOCK_BEGIN(this->packageManager)
    {
      Init();
    }
    MPM_LOCK_END();
  }
}

void PackageIteratorImpl::Init()
{
  packageManager->GetPackageDataStore()->Load();
  for (const auto& p : *packageManager->GetPackageDataStore())
  {
    snapshot.push_back(p);
  }
  iter = snapshot.begin();
}

bool PackageIteratorImpl::GetNext(PackageInfo& packageInfo)
{
  bool found = false;
  for (; !found && iter != snapshot.end(); ++iter)
  {
    if (filter[PackageFilter::Top] && iter->requiredBy.size() != 0)
    {
      continue;
    }
    if (filter[PackageFilter::Obsolete] && !iter->isObsolete)
    {
      continue;
    }
    packageInfo = *iter;
    found = true;
  }
  return found;
}

PackageIteratorImpl::~PackageIteratorImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception&)
  {
  }
}

void PackageIteratorImpl::Dispose()
{
}

PackageIterator::~PackageIterator() noexcept
{
}
