/* miktex/Core/CsvList.h:                               -*- C++ -*-

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

#if !defined(FB648E67CF8147BEB6646148F256DC30)
#define FB648E67CF8147BEB6646148F256DC30

#include <miktex/Core/config.h>

#include <memory>
#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

class CsvList
{
public:
  CsvList() = delete;

public:
  CsvList(const CsvList & other) = delete;

public:
  CsvList & operator= (const CsvList & other) = delete;

public:
  CsvList(CsvList && other) = delete;

public:
  CsvList & operator= (CsvList && other) = delete;

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~CsvList() noexcept;

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL CsvList(const std::string & s, char separator);

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL operator bool() const;

public:
  MIKTEXCORETHISAPI(std::string) operator* () const;

public:
  MIKTEXCORETHISAPI(CsvList &) operator++ ();

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
