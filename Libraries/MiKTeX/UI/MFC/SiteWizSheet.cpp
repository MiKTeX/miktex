/* SiteWizSheet.cpp:

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

#include "miktex/UI/MFC/SiteWizSheet.h"

#include "SiteWizDrive.h"
#include "SiteWizLocal.h"
#include "SiteWizRemote.h"
#include "SiteWizType.h"
#include "resource.h"

using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;

SiteWizSheetImpl::SiteWizSheetImpl(CWnd * pParent) :
  pManager(PackageManager::Create())
{
  Construct(IDS_SITEWIZ, pParent);

  pCD = new SiteWizDrive(pManager);
  pLocal = new SiteWizLocal(pManager);
  pRemote = new SiteWizRemote(pManager);
  pType = new SiteWizType(pManager);

  AddPage(pType);
  AddPage(pCD);
  AddPage(pLocal);
  AddPage(pRemote);

#if 1
  SetWizardMode();
#else
  m_psh.dwFlags |= PSH_WIZARD_LITE;
#endif

#if 0
  m_psh.dwFlags &= ~PSH_HASHELP;
#endif
}

SiteWizSheetImpl::~SiteWizSheetImpl()
{
  delete pCD;
  delete pLocal;
  delete pRemote;
  delete pType;
}

BEGIN_MESSAGE_MAP(SiteWizSheetImpl, CPropertySheet)
END_MESSAGE_MAP();

INT_PTR SiteWizSheet::DoModal(CWnd * pParent)
{
  BEGIN_USE_MY_RESOURCES()
  {
    SiteWizSheetImpl dlg(pParent);
    return dlg.DoModal();
  }
  END_USE_MY_RESOURCES();
}
