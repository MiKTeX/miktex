/* comSession.h: MiKTeX session                         -*- C++ -*-

   Copyright (C) 2006-2018 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "Resource.h"

// FIXME: must come first
#include "core-version.h"

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
namespace MiKTeXSessionLib = MAKE_CURVER_ID(MiKTeXSession);
#endif

class ATL_NO_VTABLE comSession :
  public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
  public ATL::CComCoClass<comSession, &__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(MiKTeXSession))>,
  public ISupportErrorInfo, public ATL::IDispatchImpl<MiKTeXSessionLib::ISession2, &__uuidof(MiKTeXSessionLib::ISession2), &__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(__MiKTeXSession)), /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  comSession()
  {
  }

public:
  virtual ~comSession();

public:
  static HRESULT WINAPI UpdateRegistry(BOOL doRegister);

public:
  DECLARE_CLASSFACTORY_SINGLETON(comSession);

public:
  BEGIN_COM_MAP(comSession)
    COM_INTERFACE_ENTRY(ISession)
    COM_INTERFACE_ENTRY(ISession2)
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
  STDMETHOD(RegisterRootDirectories) (BSTR rootDirectories);

public:
  STDMETHOD(FindPkFile) (BSTR fontName, BSTR mode, LONG dpi, BSTR* path, VARIANT_BOOL* found);

public:
  STDMETHOD(GetErrorInfo) (MiKTeXSessionLib::ErrorInfo* pErrorInfo);

public:
  STDMETHOD(GetMiKTeXSetupInfo) (MiKTeXSessionLib::MiKTeXSetupInfo* setupInfo);

public:
  STDMETHOD(GetRootDirectory) (LONG rootIdx, BSTR* rootDirectory);

public:
  STDMETHOD(FindFile) (BSTR fileName, BSTR* path, VARIANT_BOOL* found);

private:
  void CreateSession();

private:
  MiKTeX::Core::MiKTeXException lastMiKTeXException;

private:
  std::shared_ptr<MiKTeX::Core::Session> session;
};

OBJECT_ENTRY_AUTO(__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(MiKTeXSession)), comSession);
