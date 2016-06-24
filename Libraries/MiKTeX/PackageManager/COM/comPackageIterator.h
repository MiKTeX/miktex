/* comPackageIterator.h:                                -*- C++ -*-

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

#pragma once

#include "Resource.h"

inline void CopyPackageInfo(MiKTeXPackageManagerLib::PackageInfo & left, const MiKTeX::Packages::PackageInfo & right)
{
  left.deploymentName = _bstr_t(UW_(right.deploymentName)).Detach();
  left.displayName = _bstr_t(UW_(right.displayName)).Detach();
  left.title = _bstr_t(UW_(right.title)).Detach();
  left.version = _bstr_t(UW_(right.version)).Detach();
  left.description = _bstr_t(UW_(right.description)).Detach();
  left.creator = _bstr_t(UW_(right.creator)).Detach();
  left.sizeRunFiles = right.sizeRunFiles;
  left.sizeDocFiles = right.sizeDocFiles;
  left.sizeSourceFiles = right.sizeSourceFiles;
  left.numRunFiles = right.runFiles.size();
  left.numDocFiles = right.docFiles.size();
  left.numSourceFiles = right.sourceFiles.size();
  if (right.timePackaged == static_cast<time_t>(-1) || right.timePackaged == static_cast<time_t>(0))
  {
    left.timePackaged = ATL::COleDateTime();
  }
  else
  {
    left.timePackaged = ATL::COleDateTime(right.timePackaged);
  }
  if (right.timeInstalled == static_cast<time_t>(-1) || right.timeInstalled == static_cast<time_t>(0))
  {
    left.timeInstalled = ATL::COleDateTime();
    left.isInstalled = VARIANT_FALSE;
  }
  else
  {
    left.timeInstalled = ATL::COleDateTime(right.timeInstalled);
    left.isInstalled = VARIANT_TRUE;
  }
  left.archiveFileSize = right.archiveFileSize;
  memcpy(&left.digest, &right.digest, sizeof(left.digest));
}

inline void CopyPackageInfo(MiKTeXPackageManagerLib::PackageInfo2 & left, const MiKTeX::Packages::PackageInfo & right)
{
  left.deploymentName = _bstr_t(UW_(right.deploymentName)).Detach();
  left.displayName = _bstr_t(UW_(right.displayName)).Detach();
  left.title = _bstr_t(UW_(right.title)).Detach();
  left.version = _bstr_t(UW_(right.version)).Detach();
  left.description = _bstr_t(UW_(right.description)).Detach();
  left.creator = _bstr_t(UW_(right.creator)).Detach();
  left.sizeRunFiles = right.sizeRunFiles;
  left.sizeDocFiles = right.sizeDocFiles;
  left.sizeSourceFiles = right.sizeSourceFiles;
  left.numRunFiles = right.runFiles.size();
  left.numDocFiles = right.docFiles.size();
  left.numSourceFiles = right.sourceFiles.size();
  if (right.timePackaged == static_cast<time_t>(-1) || right.timePackaged == static_cast<time_t>(0))
  {
    left.timePackaged = ATL::COleDateTime();
  }
  else
  {
    left.timePackaged = ATL::COleDateTime(right.timePackaged);
  }
  if (right.timeInstalled == static_cast<time_t>(-1) || right.timeInstalled == static_cast<time_t>(0))
  {
    left.timeInstalled = ATL::COleDateTime();
    left.isInstalled = VARIANT_FALSE;
  }
  else
  {
    left.timeInstalled = ATL::COleDateTime(right.timeInstalled);
    left.isInstalled = VARIANT_TRUE;
  }
  left.archiveFileSize = right.archiveFileSize;
  memcpy(&left.digest, &right.digest, sizeof(left.digest));
#if MIKTEX_EXTENDED_PACKAGEINFO
  left.copyrightOwner = _bstr_t(UW_(right.copyrightOwner)).Detach();
  left.copyrightYear = _bstr_t(UW_(right.copyrightYear)).Detach();
  left.licenseType = _bstr_t(UW_(right.licenseType)).Detach();
  left.ctanPath = _bstr_t(UW_(right.ctanPath)).Detach();
#else
  left.copyrightOwner = _bstr_t("").Detach();
  left.copyrightYear = _bstr_t("").Detach();
  left.licenseType = _bstr_t("").Detach();
  left.ctanPath = _bstr_t("").Detach();
#endif
}

class ATL_NO_VTABLE comPackageIterator :
  public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
  public MiKTeXPackageManagerLib::IPackageIterator2,
  public ISupportErrorInfo
{
public:
  comPackageIterator()
  {
  }

public:
  virtual ~comPackageIterator();

public:
  BEGIN_COM_MAP(comPackageIterator)
    COM_INTERFACE_ENTRY(MiKTeXPackageManagerLib::IPackageIterator)
    COM_INTERFACE_ENTRY(MiKTeXPackageManagerLib::IPackageIterator2)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
  END_COM_MAP();

public:
  STDMETHOD(InterfaceSupportsErrorInfo) (REFIID riid);

public:
  DECLARE_PROTECT_FINAL_CONSTRUCT();

public:
  HRESULT FinalConstruct()
  {
    return S_OK;
  }

public:
  void FinalRelease();

public:
  void Initialize();

public:
  STDMETHOD(GetNextPackageInfo) (MiKTeXPackageManagerLib::PackageInfo * pPackageInfo, VARIANT_BOOL * pDone);

public:
  STDMETHOD(GetNextPackageInfo2) (MiKTeXPackageManagerLib::PackageInfo2 * pPackageInfo, VARIANT_BOOL * pDone);

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::shared_ptr<MiKTeX::Packages::PackageIterator> packageIterator;
};
