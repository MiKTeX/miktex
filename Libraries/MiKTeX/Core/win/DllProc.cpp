/* DllProc.cpp: loading DLL procedures

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/win/DllProc.h"

using namespace MiKTeX::Core;
using namespace std;

DllProcBase::DllProcBase()
{
}

DllProcBase::DllProcBase(const char * lpszDllName, const char * lpszProcName) :
  dllName(lpszDllName),
  procName(lpszProcName)
{
}

DllProcBase::~DllProcBase()
{
  try
  {
    if (hModule != nullptr && !FreeLibrary(hModule))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("FreeLibrary");
    }
  }
  catch (const exception &)
  {
  }
}

FARPROC DllProcBase::GetProc()
{
  if (proc != nullptr)
  {
    return proc;
  }
  if (hModule == nullptr)
  {
    hModule = LoadLibraryW(UW_(dllName));
    if (hModule == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("LoadLibraryW", "dllName", dllName);
    }
  }
  proc = GetProcAddress(hModule, procName.c_str());
  if (proc == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetProcAddress", "dllName", dllName, "procName", procName);
  }
  return proc;
}
