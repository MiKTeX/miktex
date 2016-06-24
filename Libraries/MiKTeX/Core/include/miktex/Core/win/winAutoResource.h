/* miktex/Core/win/AutoResource.h:                      -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(CDCDDDEC5C1041FFB4A5B77E76755DDA)
#define CDCDDDEC5C1041FFB4A5B77E76755DDA

#include <miktex/Core/config.h>

#include <Windows.h>

#include "../AutoResource.h"
#include "../Session.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

struct SetErrorMode_
{
public:
  void operator() (unsigned int mode) const
  {
    SetErrorMode(mode);
  }
};

typedef AutoResource<unsigned int, SetErrorMode_> AutoErrorMode;

template<> inline HANDLE InvalidHandleValue<HANDLE>()
{
  return INVALID_HANDLE_VALUE;
}

struct CloseHandle_
{
public:
  void operator() (HANDLE h) const
  {
    if (!CloseHandle(h))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
    }
  }
};

typedef AutoResource<HANDLE, CloseHandle_> AutoHANDLE;

struct RegCloseKey_
{
public:
  void operator() (HKEY hkey) const
  {
    long result = RegCloseKey(hkey);
    if (result != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("RegCloseKey");
    }
  }
};

typedef AutoResource<HKEY, RegCloseKey_> AutoHKEY;

struct GlobalUnlock_
{
public:
  void operator() (HGLOBAL hMem) const
  {
    if (GlobalUnlock(hMem) == FALSE && GetLastError() != NO_ERROR)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GlobalUnlock");
    }
  }
};

typedef AutoResource<void *, GlobalUnlock_> AutoGlobal;

struct GlobalFree_
{
public:
  void operator() (HGLOBAL hMem) const
  {
    if (GlobalFree(hMem) != nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GlobalFree");
    }
  }
};

typedef AutoResource<void *, GlobalFree_> AutoGlobalMemory;

struct LocalFree_
{
public:
  void operator() (void * ptr) const
  {
    if (LocalFree(ptr) != nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("LocalFree");
    }
  }
};

typedef AutoResource<void *, LocalFree_> AutoLocalMemory;
typedef AutoResource<void *, LocalFree_> AutoLocalMem;

#if defined(WINOLEAPI)
struct CoTaskMemFree_
{
public:
  void operator() (void * ptr) const
  {
    CoTaskMemFree(ptr);
  }
};

typedef AutoResource<void *, CoTaskMemFree_> AutoCoTaskMem;
#endif

#if defined(WINOLEAUTAPI)
struct SysFreeString_
{
public:
  void operator() (BSTR bstr) const
  {
    SysFreeString(bstr);
  }
};

typedef AutoResource<BSTR, SysFreeString_> AutoSysString;
#endif

#if defined(OpenPrinter)
struct ClosePrinter_
{
public:
  void operator() (HANDLE hPrinter) const
  {
    if (!ClosePrinter(hPrinter))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Closeprinter");
    }
  }
};

typedef AutoResource<HANDLE, ClosePrinter_> AutoClosePrinter;
#endif

#if defined(DdeCreateStringHandle)
struct DdeFreeStringHandle_
{
public:
  void operator() (unsigned long inst, HSZ hsz) const
  {
    if (!DdeFreeStringHandle(inst, hsz))
    {
    }
  }
};

typedef AutoResource2<unsigned long, HSZ, DdeFreeStringHandle_> AutoDdeFreeStringHandle;
#endif

struct FreeSid_
{
public:
  void operator() (PSID psid) const
  {
    if (FreeSid(psid) != nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("FreeSid");
    }
  }
};

typedef MiKTeX::Core::AutoResource<PSID, FreeSid_> AutoSid;

MIKTEX_CORE_END_NAMESPACE;

#endif
