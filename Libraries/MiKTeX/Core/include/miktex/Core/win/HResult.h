/* miktex/Core/HResult.h:                               -*- C++ -*-

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

#if !defined(AD4328AD75F0402480253D8834185B23)
#define AD4328AD75F0402480253D8834185B23

#include <miktex/Core/config.h>

#include <Windows.h>

#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

class HResult
{
public:
  HResult()
  {
  }

public:
  HResult(HRESULT hr) :
    hr(hr)
  {
  }

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~HResult();

public:
  MIKTEXCORETHISAPI(std::string) ToString() const;

public:
  long GetCode() const
  {
    return HRESULT_CODE(hr);
  }

public:
  long GetFacility() const
  {
    return HRESULT_FACILITY(hr);
  }

public:
  long GetSeverity() const
  {
    return HRESULT_SEVERITY(hr);
  }

public:
  bool Failed() const
  {
    return FAILED(hr);
  }

public:
  bool Succeeded() const
  {
    return SUCCEEDED(hr);
  }

public:
  bool operator== (HRESULT other)
  {
    return hr == other;
  }

public:
  // FIXME: use wchar_t pointer
  MIKTEXCORETHISAPI(const char *) GetText();

private:
  // FIXME: use wchar_t pointer
  char * lpszMessage = nullptr;

private:
  HRESULT hr = S_FALSE;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
