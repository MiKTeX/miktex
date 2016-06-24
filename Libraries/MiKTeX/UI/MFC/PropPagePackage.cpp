/* PropPagePackage.cpp:

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

#include "PropPagePackage.h"
#include "resource.h"

using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace std;

static void MakeOneLine(CString & dest, const wstring & source)
{
  for (wstring::const_iterator it = source.begin(); it != source.end(); ++it)
  {
    if (*it == L'\n')
    {
      dest += L' ';
    }
    else
    {
      dest += *it;
    }
  }
}

PropPagePackage::PropPagePackage(const PackageInfo & packageInfo) :
  packageInfo(packageInfo)
{
  Construct(IDD_PROPPAGE_PACKAGE);
  m_psp.dwFlags &= ~(PSP_HASHELP);
  date = CTime(packageInfo.timePackaged).Format(_T("%Y-%m-%d %H:%M:%S"));
  MakeOneLine(description, StringUtil::UTF8ToWideChar(packageInfo.description));
  name = StringUtil::UTF8ToWideChar(packageInfo.displayName).c_str();
  MakeOneLine(title, StringUtil::UTF8ToWideChar(packageInfo.title));
  size.Format(T_(_T("%u Bytes")), static_cast<unsigned>(packageInfo.GetSize()));
  maintainer = StringUtil::UTF8ToWideChar(packageInfo.copyrightOwner).c_str();
  version = StringUtil::UTF8ToWideChar(packageInfo.version).c_str();
}

void PropPagePackage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_DATE, date);
  DDX_Text(pDX, IDC_DESCRIPTION, description);
  DDX_Text(pDX, IDC_MAINTAINER, maintainer);
  DDX_Text(pDX, IDC_NAME, name);
  DDX_Text(pDX, IDC_PACKAGE_SIZE, size);
  DDX_Text(pDX, IDC_TITLE, title);
  DDX_Text(pDX, IDC_VERSION, version);
}

BEGIN_MESSAGE_MAP(PropPagePackage, CPropertyPage)
END_MESSAGE_MAP();

BOOL PropPagePackage::OnInitDialog()
{
  return CPropertyPage::OnInitDialog();
}
