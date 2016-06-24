/* session.cpp: MiKTeX session

   Copyright (C) 2006-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "Resource.h"

#include "COM/session.h"
#include "Session/SessionImpl.h"

MiKTeXSessionModule _AtlModule;

using namespace MiKTeX::Core;

BOOL SessionImpl::AtlDllMain(DWORD reason, LPVOID lpReserved)
{
  return _AtlModule.DllMain(reason, lpReserved);
}

STDAPI DllCanUnloadNow()
{
  return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
  return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
#if REPORT_EVENTS
  if (!AddEventSource())
  {
    return E_FAIL;
  }
#endif
  HRESULT hr = _AtlModule.DllRegisterServer();
  DbgView("DllRegisterServer() " + SUCCEEDED(hr) ? "succeeded" : "failed");
  return hr;
}

STDAPI DllUnregisterServer()
{
#if REPORT_EVENTS
  if (RemoveEventSource())
  {
    return E_FAIL;
  }
#endif
  HRESULT hr = _AtlModule.DllUnregisterServer();
  DbgView("DllUnregisterServer() " + SUCCEEDED(hr) ? "succeeded" : "failed");
  return hr;
}
