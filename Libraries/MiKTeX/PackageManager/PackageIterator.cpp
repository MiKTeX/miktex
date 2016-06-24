/* PackageIterator.cpp:

   Copyright (C) 2001-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

using namespace MiKTeX::Packages;
using namespace std;

PackageIteratorImpl::PackageIteratorImpl(shared_ptr<PackageManagerImpl> pManager) :
  pManager(pManager)
{
  pManager->GetAllPackageDefinitions(snapshot);
  iter = snapshot.begin();
}

bool PackageIteratorImpl::GetNext(PackageInfo & packageInfo)
{
  bool found = false;
  for (; !found && iter != snapshot.end(); ++iter)
  {
    if (filter[PackageFilter::Top] && iter->requiredBy.size() != 0)
    {
      continue;
    }
    if (filter[PackageFilter::RequiredBy]
      && (find(iter->requiredBy.begin(), iter->requiredBy.end(), requiredBy) == iter->requiredBy.end()))
    {
      continue;
    }
    if (filter[PackageFilter::Obsolete] && !pManager->IsPackageObsolete(iter->deploymentName))
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
  catch (const exception &)
  {
  }
}

void PackageIteratorImpl::Dispose()
{
}

PackageIterator::~PackageIterator()
{
}
