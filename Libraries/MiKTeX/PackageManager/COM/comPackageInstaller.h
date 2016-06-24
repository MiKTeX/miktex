/* PackageInstaller.h:                                  -*- C++ -*-

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
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "Resource.h"

class ATL_NO_VTABLE comPackageInstaller :
  public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
  public ISupportErrorInfo,
  public ATL::IDispatchImpl<MiKTeXPackageManagerLib::IPackageInstaller, &__uuidof(MiKTeXPackageManagerLib::IPackageInstaller), &__uuidof(MiKTeXPackageManagerLib::MAKE_CURVER_ID(__MiKTeXPackageManager)), /*wMajor =*/ 1, /*wMinor =*/ 0>,
  public MiKTeX::Packages::PackageInstallerCallback
{
public:
  comPackageInstaller()
  {
  }

public:
  virtual ~comPackageInstaller();

public:
  BEGIN_COM_MAP(comPackageInstaller)
    COM_INTERFACE_ENTRY(IPackageInstaller)
    COM_INTERFACE_ENTRY(IDispatch)
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
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(MiKTeX::Packages::Notification nf);

public:
  STDMETHOD(Add) (BSTR packageName, VARIANT_BOOL toBeInstalled);

public:
  STDMETHOD(SetCallback) (IUnknown * pCallback);

public:
  STDMETHOD(InstallRemove) ();

public:
  STDMETHOD(GetErrorInfo) (MiKTeXPackageManagerLib::ErrorInfo * pErrorInfo);

public:
  STDMETHOD(UpdateDb) ();

public:
  STDMETHOD(SetRepository) (BSTR repository);

private:
  std::vector<std::string> packagesToBeInstalled;

private:
  std::vector<std::string> packagesToBeRemoved;

private:
  ATL::CComQIPtr<MiKTeXPackageManagerLib::IPackageInstallerCallback> installerCallback;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;

private:
  MiKTeX::Core::MiKTeXException lastMiKTeXException;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
};
