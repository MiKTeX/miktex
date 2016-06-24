/* comPackageInstaller.cpp:

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

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeXPackageManagerLib;
using namespace std;

comPackageInstaller::~comPackageInstaller()
{
  try
  {
    if (trace_mpm != nullptr)
    {
      trace_mpm->Close();
      trace_mpm = nullptr;
    }
    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

void comPackageInstaller::FinalRelease()
{
}

void comPackageInstaller::Initialize()
{
  if (trace_error == nullptr)
  {
    trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
  }
  if (trace_mpm == nullptr)
  {
    trace_mpm = TraceStream::Open(MIKTEX_TRACE_MPM);
  }
}

STDMETHODIMP comPackageInstaller::InterfaceSupportsErrorInfo(REFIID riid)
{
  static const IID * const interfaces[] =
  {
    &__uuidof(IPackageInstaller)
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

void comPackageInstaller::ReportLine(const string & str)
{
  if (installerCallback == nullptr)
  {
    return;
  }
  HRESULT hr = installerCallback->ReportLine(_bstr_t(StringUtil::UTF8ToWideChar(str).c_str()));
  if (FAILED(hr))
  {
    // FIXME
  }
}

bool comPackageInstaller::OnRetryableError(const string & message)
{
  if (installerCallback == nullptr)
  {
    return false;
  }
  VARIANT_BOOL doContinue;
  HRESULT hr = installerCallback->OnRetryableError(_bstr_t(StringUtil::UTF8ToWideChar(message).c_str()), &doContinue);
  if (FAILED(hr))
  {
    doContinue = VARIANT_FALSE;
  }
  return doContinue ? true : false;
}

bool comPackageInstaller::OnProgress(Notification nf)
{
  if (installerCallback == nullptr)
  {
    return true;
  }
  VARIANT_BOOL doContinue;
  HRESULT hr = installerCallback->OnProgress((long)nf, &doContinue);
  if (FAILED(hr))
  {
    doContinue = VARIANT_FALSE;
  }
  return doContinue ? true : false;
}

STDMETHODIMP comPackageInstaller::Add(BSTR packageName, VARIANT_BOOL toBeInstalled)
{
  HRESULT hr = S_OK;
  try
  {
    if (toBeInstalled)
    {
      packagesToBeInstalled.push_back(WU_(packageName));
      trace_mpm->WriteFormattedLine("mpmsvc", T_("to be installed: %s"), packagesToBeInstalled.back().c_str());
    }
    else
    {
      packagesToBeRemoved.push_back(WU_(packageName));
      trace_mpm->WriteFormattedLine("mpmsvc", T_("to be removed: %s"), packagesToBeRemoved.back().c_str());
    }
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("mpmsvc", e.what(), MiKTeXException::KVMAP(), SourceLocation());
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comPackageInstaller::SetCallback(IUnknown * installerCallback)
{
  this->installerCallback = installerCallback;
  return S_OK;
}

STDMETHODIMP comPackageInstaller::InstallRemove()
{
  HRESULT hr = S_OK;
  try
  {
    trace_mpm->WriteLine("mpmsvc", T_("install/remove"));
    if (packageInstaller == nullptr)
    {
      if (packageManager == nullptr)
      {
        packageManager = PackageManager::Create();
      }
      packageInstaller = packageManager->CreateInstaller();
    }
    packageInstaller->SetCallback(this);
    packageInstaller->SetFileLists(packagesToBeInstalled, packagesToBeRemoved);
    packageInstaller->InstallRemove();
    packagesToBeInstalled.clear();
    packagesToBeRemoved.clear();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("mpmsvc", e.what(), MiKTeXException::KVMAP(), SourceLocation());
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comPackageInstaller::GetErrorInfo(ErrorInfo * pErrorInfo)
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

STDMETHODIMP comPackageInstaller::UpdateDb()
{
  HRESULT hr = S_OK;
  try
  {
    trace_mpm->WriteLine("mpmsvc", T_("update db"));
    if (packageInstaller == nullptr)
    {
      if (packageManager == nullptr)
      {
        packageManager = PackageManager::Create();
      }
      packageInstaller = packageManager->CreateInstaller();
    }
    packageInstaller->UpdateDb();
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("mpmsvc", e.what(), MiKTeXException::KVMAP(), SourceLocation());
    hr = E_FAIL;
  }
  return hr;
}

STDMETHODIMP comPackageInstaller::SetRepository(BSTR repository)
{
  HRESULT hr = S_OK;
  try
  {
    trace_mpm->WriteLine("mpmsvc", T_("set repository"));
    if (packageInstaller == nullptr)
    {
      if (packageManager == nullptr)
      {
        packageManager = PackageManager::Create();
      }
      packageInstaller = packageManager->CreateInstaller();
    }
    packageInstaller->SetRepository(WU_(repository));
  }
  catch (const MiKTeXException & e)
  {
    lastMiKTeXException = e;
    hr = E_FAIL;
  }
  catch (const exception & e)
  {
    lastMiKTeXException = MiKTeXException("mpmsvc", e.what(), MiKTeXException::KVMAP(), SourceLocation());
    hr = E_FAIL;
  }
  return hr;
}
