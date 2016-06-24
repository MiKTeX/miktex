/* SiteWizDrive.cpp:

   Copyright (C) 2008-2016 Christian Schenk

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

#include "miktex/UI/Qt/ErrorDialog.h"

#include "SiteWizSheet.h"
#include "SiteWizDrive.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

SiteWizDrive::SiteWizDrive(shared_ptr<PackageManager> pManager) :
  QWizardPage(nullptr),
  pManager(pManager)
{
  setupUi(this);
}

void SiteWizDrive::initializePage()
{
  try
  {
    if (!firstVisit)
    {
      return;
    }
    firstVisit = false;
#if defined(MIKTEX_WINDOWS)
    FindMiKTeXCDs(locations);
#endif
    cbDrives->clear();
    for (vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
      cbDrives->addItem(QString::fromUtf8(it->description.c_str()));
    }
    if (locations.empty())
    {
      cbDrives->addItem(T_("No MiKTeX CD/DVD found"));
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

bool SiteWizDrive::isComplete() const
{
  return !locations.empty();
}

bool SiteWizDrive::validatePage()
{
  try
  {
    if (!QWizardPage::validatePage())
    {
      return false;
    }
    pManager->SetDefaultPackageRepository(RepositoryType::MiKTeXDirect, locations[cbDrives->currentIndex()].directory);
    return true;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return false;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return false;
  }
}

#if defined(MIKTEX_WINDOWS)
void SiteWizDrive::FindMiKTeXCDs(vector<SiteWizDrive::Location> & locations)
{
  AutoErrorMode autoMode(SetErrorMode(SEM_FAILCRITICALERRORS));

  DWORD logicalDrives = GetLogicalDrives();
  for (int drv = 0; logicalDrives != 0; logicalDrives >>= 1, ++drv)
  {
    if ((logicalDrives & 1) == 0)
    {
      continue;
    }

    wchar_t szDrive[4];
    szDrive[0] = L'A' + static_cast<wchar_t>(drv);
    szDrive[1] = L':';
    szDrive[2] = L'\\';
    szDrive[3] = 0;

    DWORD fileSystemFlags;
    DWORD maximumComponentLength;
    wchar_t fileSystemName[BufferSizes::MaxPath];
    wchar_t volumeName[BufferSizes::MaxPath];

    if (!GetVolumeInformationW(szDrive, volumeName, BufferSizes::MaxPath, nullptr, &maximumComponentLength, &fileSystemFlags, fileSystemName, BufferSizes::MaxPath))
    {
      continue;
    }

    if (!Utils::IsMiKTeXDirectRoot(szDrive))
    {
      continue;
    }

    Location location;
    location.directory = WU_(szDrive);

    szDrive[2] = 0;
    location.description = WU_(szDrive);
    location.description += " (";
    location.description += WU_(volumeName);
    location.description += ')';

    locations.push_back(location);
  }
}
#endif
