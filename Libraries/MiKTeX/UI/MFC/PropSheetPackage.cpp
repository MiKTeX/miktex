/* PropSheetPackage.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/MFC/PropSheetPackage.h"

#include "PropPageFiles.h"
#include "PropPagePackage.h"
#include "resource.h"

using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;

class PropSheetPackageImpl : public CPropertySheet
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  PropSheetPackageImpl(const PackageInfo & packageInfo, CWnd * pParentWnd = nullptr, UINT selectPage = 0);

public:
  virtual ~PropSheetPackageImpl();

protected:
  CPropertyPage * pPage1 = nullptr;

protected:
  CPropertyPage * pPage2 = nullptr;
};

PropSheetPackageImpl::PropSheetPackageImpl(const PackageInfo & packageInfo, CWnd * pParentWnd, UINT selectPage)
{
  Construct(UT_(packageInfo.deploymentName), pParentWnd, selectPage);
  pPage1 = new PropPagePackage(packageInfo);
  pPage2 = new PropPageFiles(packageInfo);
  AddPage(pPage1);
  AddPage(pPage2);
}

PropSheetPackageImpl::~PropSheetPackageImpl()
{
  delete pPage1;
  delete pPage2;
}

BEGIN_MESSAGE_MAP(PropSheetPackageImpl, CPropertySheet)
END_MESSAGE_MAP();

INT_PTR PropSheetPackage::DoModal(const PackageInfo & packageInfo, CWnd * pParentWnd)
{
  BEGIN_USE_MY_RESOURCES()
  {
    PropSheetPackageImpl dlg(packageInfo, pParentWnd);
    return dlg.DoModal();
  }
  END_USE_MY_RESOURCES()
}
