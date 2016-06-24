/* comSession.cpp: MiKTeX session

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

#include "miktex/Core/win/winAutoResource.h"

#include "COM/comSession.h"
#include "COM/session.h"
#include "Session/SessionImpl.h"

using namespace ATL;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeXSessionLib;
using namespace std;

comSession::~comSession()
{
  try
  {
    session = nullptr;
  }
  catch (const exception &)
  {
  }
}

void comSession::FinalRelease()
{
  try
  {
    session = nullptr;
  }
  catch (const exception &)
  {
  }
}

STDMETHODIMP comSession::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID * const interfaces[] =
  {
    &__uuidof(ISession),
    &__uuidof(ISession2)
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

STDMETHODIMP comSession::RegisterRootDirectories(BSTR rootDirectories)
{
  SessionImpl::runningAsLocalServer = true;
  HRESULT hr = S_OK;
  try
  {
    CreateSession();
    session->RegisterRootDirectories(WU_(rootDirectories));
  }
  catch (const _com_error & e)
  {
    hr = e.Error();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("sessionsvc", e.what(), nullptr, __FILE__, __LINE__);
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comSession::FindPkFile(BSTR fontName, BSTR mode, LONG dpi, BSTR * path, VARIANT_BOOL * found)
{
  SessionImpl::runningAsLocalServer = true;
  HRESULT hr = S_OK;
  try
  {
    CreateSession();
    PathName path_;
    if (session->FindPkFile(WU_(fontName), WU_(mode), dpi, path_))
    {
      *path = _bstr_t(path_.ToWideCharString().c_str()).Detach();
      *found = VARIANT_TRUE;
    }
    else
    {
      *path = _bstr_t(L"").Detach();
      *found = VARIANT_FALSE;
    }
  }
  catch (const _com_error & e)
  {
    hr = e.Error();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("sessionsvc", e.what(), nullptr, __FILE__, __LINE__);
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comSession::GetErrorInfo(ErrorInfo * pErrorInfo)
{
  if (lastMiKTeXException.what() == nullptr)
  {
    return E_FAIL;
  }
  try
  {
    _bstr_t message = UW_(lastMiKTeXException.what());
    _bstr_t info = UW_(lastMiKTeXException.GetInfo());
    _bstr_t sourceFile = UW_(lastMiKTeXException.GetSourceFile());
    pErrorInfo->message = message.Detach();
    pErrorInfo->info = info.Detach();
    pErrorInfo->sourceFile = sourceFile.Detach();
    pErrorInfo->sourceLine = lastMiKTeXException.GetSourceLine();
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

STDMETHODIMP comSession::GetMiKTeXSetupInfo(MiKTeXSetupInfo * setupInfo)
{
  SessionImpl::runningAsLocalServer = true;
  HRESULT hr = S_OK;
  try
  {
    CreateSession();
    _bstr_t version = UW_(Utils::GetMiKTeXVersionString());
    _bstr_t binDirectory = UW_(session->GetSpecialPath(SpecialPath::BinDirectory).ToWideCharString());
    _bstr_t installRoot = UW_(session->GetSpecialPath(SpecialPath::InstallRoot).ToWideCharString());
    _bstr_t commonConfigRoot;
    _bstr_t commonDataRoot;
    commonConfigRoot = UW_(session->GetSpecialPath(SpecialPath::CommonConfigRoot).ToWideCharString());
    commonDataRoot = UW_(session->GetSpecialPath(SpecialPath::CommonDataRoot).ToWideCharString());
    _bstr_t userConfigRoot = UW_(session->GetSpecialPath(SpecialPath::UserConfigRoot).ToWideCharString());
    _bstr_t userDataRoot = UW_(session->GetSpecialPath(SpecialPath::UserDataRoot).ToWideCharString());
    setupInfo->sharedSetup = TriState::Undetermined == TriState::True ? VARIANT_TRUE : VARIANT_FALSE;
    setupInfo->series = MIKTEX_SERIES_INT;
    setupInfo->numRoots = session->GetNumberOfTEXMFRoots();
    setupInfo->version = version.Detach();
    setupInfo->binDirectory = binDirectory.Detach();
    setupInfo->installRoot = installRoot.Detach();
    setupInfo->commonConfigRoot = commonConfigRoot.Detach();
    setupInfo->commonDataRoot = commonDataRoot.Detach();
    setupInfo->userConfigRoot = userConfigRoot.Detach();
    setupInfo->userDataRoot = userDataRoot.Detach();
    return S_OK;

  }
  catch (const _com_error & e)
  {
    hr = e.Error();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("sessionsvc", e.what(), nullptr, __FILE__, __LINE__);
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comSession::GetRootDirectory(LONG rootIdx, BSTR * rootDirectory)
{
  SessionImpl::runningAsLocalServer = true;
  HRESULT hr = S_OK;
  try
  {
    CreateSession();
    *rootDirectory = _bstr_t(session->GetRootDirectory(rootIdx).ToWideCharString().c_str()).Detach();
    return S_OK;
  }
  catch (const _com_error & e)
  {
    hr = e.Error();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("sessionsvc", e.what(), nullptr, __FILE__, __LINE__);
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comSession::FindFile(BSTR fileName, BSTR * path, VARIANT_BOOL * found)
{
  SessionImpl::runningAsLocalServer = true;
  HRESULT hr = S_OK;
  try
  {
    CreateSession();
    FileType fileType = session->DeriveFileType(WU_(fileName));
    if (fileType == FileType::None)
    {
      fileType = FileType::TEX;
    }
    PathName path_;
    if (session->FindFile(WU_(fileName), fileType, path_))
    {
      *path = _bstr_t(path_.ToWideCharString().c_str()).Detach();
      *found = VARIANT_TRUE;
    }
    else
    {
      *path = _bstr_t(L"").Detach();
      *found = VARIANT_FALSE;
    }
  }
  catch (const _com_error & e)
  {
    hr = e.Error();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("sessionsvc", e.what(), nullptr, __FILE__, __LINE__);
    hr = E_FAIL;
  }
  return hr;
}

void comSession::CreateSession()
{
  if (session == nullptr)
  {
    session = Session::TryGet();
    if (session == nullptr)
    {
      // we are running standalone; create a new session
      Session::InitInfo initInfo(SESSIONSVC);
      session = Session::Create(initInfo);
      if (session->IsSharedSetup() && session->RunningAsAdministrator())
      {
        session->SetAdminMode(true);
      }
    }
  }
}

// see MSDN "The COM Elevation Moniker"
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

HRESULT WINAPI comSession::UpdateRegistry(BOOL doRegister)
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
    hr = _AtlModule.UpdateRegistryFromResourceD(IDR_MIKTEXSESSIONOBJECT, doRegister, &regMapEntries[0]);
#else
    hr = _AtlModule.UpdateRegistryFromResource(IDR_MIKTEXSESSIONOBJECT, doRegister, &regMapEntries[0]);
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
