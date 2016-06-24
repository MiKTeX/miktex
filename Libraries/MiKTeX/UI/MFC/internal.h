/* internal.h: internal definitions			-*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX UI Library.

   MiKTeX UI Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX UI Library; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#define MIKTEXUIEXPORT __declspec(dllexport)

#define AD234BEB3E674BA7A4B3A5B557918D53
#include "miktex/UI/MFC/Prototypes.h"

#include <miktex/UI/UI>

#define T_(x) MIKTEXTEXT(x)

#if !defined(UNICODE)
#  error UNICODE required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

class USE_MY_RESOURCES
{
public:
  USE_MY_RESOURCES() :
    defaultResourceHandle(AfxGetResourceHandle())
  {
    extern AFX_EXTENSION_MODULE MikuiDLL;
    AfxSetResourceHandle(MikuiDLL.hModule);
  }

public:
  ~USE_MY_RESOURCES()
  {
    AfxSetResourceHandle(defaultResourceHandle);
  }

private:
  HINSTANCE defaultResourceHandle;
};

class SiteWizSheetImpl : public CPropertySheet
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  SiteWizSheetImpl(CWnd * pParent = nullptr);

public:
  ~SiteWizSheetImpl() override;

private:
  CPropertyPage * pType = nullptr;

private:
  CPropertyPage * pRemote = nullptr;

private:
  CPropertyPage * pLocal = nullptr;

private:
  CPropertyPage * pCD = nullptr;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState;

public:
  MiKTeX::Packages::RepositoryReleaseState GetRepositoryReleaseState() const
  {
    return repositoryReleaseState;
  }

public:
  void SetRepositoryReleaseState(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState)
  {
    this->repositoryReleaseState = repositoryReleaseState;
  }
};

#define BEGIN_USE_MY_RESOURCES() { USE_MY_RESOURCES useMyResources;
#define END_USE_MY_RESOURCES() }
