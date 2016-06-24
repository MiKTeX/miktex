/* comPackageIterator.cpp:

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
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "COM/comPackageIterator.h"

using namespace MiKTeX::Packages;
using namespace MiKTeXPackageManagerLib;

comPackageIterator::~comPackageIterator()
{
}

void comPackageIterator::FinalRelease()
{
}

void comPackageIterator::Initialize()
{
  if (packageManager == nullptr)
  {
    packageManager = PackageManager::Create();
  }
  packageIterator = packageManager->CreateIterator();
}

STDMETHODIMP comPackageIterator::GetNextPackageInfo(MiKTeXPackageManagerLib::PackageInfo * pPackageInfo, VARIANT_BOOL * pDone)
{
  *pDone = VARIANT_FALSE;
  MiKTeX::Packages::PackageInfo packageInfo;
  while (*pDone == VARIANT_FALSE && packageIterator->GetNext(packageInfo))
  {
    if (!packageInfo.IsPureContainer())
    {
      CopyPackageInfo(*pPackageInfo, packageInfo);
      *pDone = VARIANT_TRUE;
    }
  }
  return *pDone == VARIANT_FALSE ? S_FALSE : S_OK;
}

STDMETHODIMP comPackageIterator::GetNextPackageInfo2(MiKTeXPackageManagerLib::PackageInfo2 * pPackageInfo, VARIANT_BOOL * pDone)
{
  *pDone = VARIANT_FALSE;
  MiKTeX::Packages::PackageInfo packageInfo;
  while (*pDone == VARIANT_FALSE && packageIterator->GetNext(packageInfo))
  {
    if (!packageInfo.IsPureContainer())
    {
      CopyPackageInfo(*pPackageInfo, packageInfo);
      *pDone = VARIANT_TRUE;
    }
  }
  return *pDone == VARIANT_FALSE ? S_FALSE : S_OK;
}

STDMETHODIMP comPackageIterator::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID * const interfaces[] =
  {
    &__uuidof(IPackageIterator),
    &__uuidof(IPackageIterator2)
  };
  for (const IID * iid : interfaces)
  {
    if (InlineIsEqualGUID(*iid, riid))
    {
      return S_OK;
    }
  }
  return S_FALSE;
}
