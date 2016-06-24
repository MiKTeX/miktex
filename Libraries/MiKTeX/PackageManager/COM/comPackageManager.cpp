/* comPackageManager.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#include "COM/comPackageInstaller.h"
#include "COM/comPackageIterator.h"
#include "COM/comPackageManager.h"
#include "COM/mpm.h"

using namespace ATL;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeXPackageManagerLib;
using namespace std;

using namespace MiKTeXPackageManagerLib;

comPackageManager::~comPackageManager()
{
  try
  {
    session = nullptr;
  }
  catch (const exception &)
  {
  }
}

void comPackageManager::FinalRelease()
{
  try
  {
    session = nullptr;
  }
  catch (const exception &)
  {
  }
}

STDMETHODIMP comPackageManager::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID * const interfaces[] =
  {
    &__uuidof(IPackageManager),
    &__uuidof(IPackageManager2),
    &__uuidof(IPackageManager3)
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

STDMETHODIMP comPackageManager::CreateInstaller(IPackageInstaller ** ppInstaller)
{
  PackageManagerImpl::localServer = true;
  try
  {
    // create the IPackageInstaller object
    CComObject<comPackageInstaller> * pInstaller = nullptr;
    HRESULT hr = CComObject<comPackageInstaller>::CreateInstance(&pInstaller);
    if (FAILED(hr))
    {
      *ppInstaller = nullptr;
      return hr;
    }

    // increment the reference count of the new object; decrement it
    // at the end of the block
    CComPtr<IUnknown> pUnk(pInstaller->GetUnknown());

    CreateSession();

    pInstaller->Initialize();

    // return the IPackageInstaller interface
    return pUnk->QueryInterface(ppInstaller);
  }
  catch (const exception &)
  {
    *ppInstaller = nullptr;
    return E_FAIL;
  }
}

STDMETHODIMP comPackageManager::GetPackageInfo(BSTR deploymentName, MiKTeXPackageManagerLib::PackageInfo * pPackageInfo)
{
  PackageManagerImpl::localServer = true;
  try
  {
    CreateSession();
    if (packageManager == nullptr)
    {
      packageManager = MiKTeX::Packages::PackageManager::Create();
    }
    MiKTeX::Packages::PackageInfo packageInfo = packageManager->GetPackageInfo(WU_((deploymentName)));
    CopyPackageInfo(*pPackageInfo, packageInfo);
    return S_OK;
  }
  catch (const _com_error & e)
  {
    return e.Error();
  }
  catch (const exception &)
  {
    return E_FAIL;
  }
}

STDMETHODIMP comPackageManager::CreatePackageIterator(IPackageIterator ** ppIter)
{
  PackageManagerImpl::localServer = true;
  try
  {
    // create the IPackageIterator object
    CComObject<comPackageIterator> * pIter = nullptr;
    HRESULT hr = CComObject<comPackageIterator>::CreateInstance(&pIter);
    if (FAILED(hr))
    {
      *ppIter = nullptr;
      return hr;
    }

    // increment the reference count of the new object; decrement it
    // at the end of the block
    CComPtr<IUnknown> pUnk(pIter->GetUnknown());

    CreateSession();

    pIter->Initialize();

    // return the IPackageIterator interface
    return pUnk->QueryInterface(ppIter);
  }
  catch (const exception &)
  {
    *ppIter = nullptr;
    return E_FAIL;
  }
}

STDMETHODIMP comPackageManager::GetPackageInfo2(BSTR deploymentName, PackageInfo2 * pPackageInfo)
{
  PackageManagerImpl::localServer = true;
  try
  {
    CreateSession();
    if (packageManager == nullptr)
    {
      packageManager = MiKTeX::Packages::PackageManager::Create();
    }
    MiKTeX::Packages::PackageInfo packageInfo = packageManager->GetPackageInfo(WU_(deploymentName));
    CopyPackageInfo(*pPackageInfo, packageInfo);
    return S_OK;
  }
  catch (const _com_error & e)
  {
    return e.Error();
  }
  catch (const exception &)
  {
    return E_FAIL;
  }
}

void comPackageManager::CreateSession()
{
  if (session == nullptr)
  {
    session = Session::TryGet();
    if (session == nullptr)
    {
      // we are running standalone; create a new admin session
      Session::InitInfo initInfo(MPMSVC);
      session = Session::Create(initInfo);
      if (session->IsSharedSetup() && session->RunningAsAdministrator())
      {
        session->SetAdminMode(true);
      }
    }
  }
}

static ULONG GetAccessPermissionsForLUAServer(SECURITY_DESCRIPTOR ** ppSD)
{
  LPWSTR lpszSDDL = L"O:BAG:BAD:(A;;0x3;;;IU)(A;;0x3;;;SY)";
  SECURITY_DESCRIPTOR * pSD = nullptr;
  ULONG size = 0;
  if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(lpszSDDL, SDDL_REVISION_1, reinterpret_cast<PSECURITY_DESCRIPTOR *>(&pSD), &size))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("ConvertStringSecurityDescriptorToSecurityDescriptorW");
  }
  *ppSD = pSD;
  return size;
}

HRESULT WINAPI comPackageManager::UpdateRegistry(BOOL doRegister)
{
  HRESULT hr;
  try
  {
    vector<_ATL_REGMAP_ENTRY> regMapEntries;
    _ATL_REGMAP_ENTRY rme;
    SECURITY_DESCRIPTOR * pSd;
    ULONG sizeSd = GetAccessPermissionsForLUAServer(&pSd);
    AutoLocalMem toBeFreed(pSd);
    rme.szKey = L"ACCESS_SD";
    CharBuffer<wchar_t> dataString(Utils::Hexify(pSd, sizeSd, true));
    rme.szData = dataString.GetData();
    regMapEntries.push_back(rme);
    rme.szKey = nullptr;
    rme.szData = nullptr;
    regMapEntries.push_back(rme);
#if defined(_ATL_DLL)
    hr = _AtlModule.UpdateRegistryFromResourceD(IDR_PACKAGEMANAGER, doRegister, &regMapEntries[0]);
#else
    hr = _AtlModule.UpdateRegistryFromResource(IDR_PACKAGEMANAGER, doRegister, &regMapEntries[0]);
#endif
    if (FAILED(hr))
    {
      //
    }
  }
  catch (const exception &)
  {
    hr = E_FAIL;
  }
  return hr;
}
