/* miktex/Core/TemporaryDirectory.h:                    -*- C++ -*-

   Copyright (C) 2016 Christian Schenk

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

#if !defined(C0D7704DBE834212B4AE53EC5EBC7E3C)
#define C0D7704DBE834212B4AE53EC5EBC7E3C

#include <miktex/Core/config.h>

#include <memory>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE TemporaryDirectory
{
public:
  virtual MIKTEXTHISCALL ~TemporaryDirectory() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL Delete() = 0;

public:
  virtual PathName MIKTEXTHISCALL GetPathName() = 0;

public:
  static MIKTEXCORECEEAPI(std::unique_ptr<TemporaryDirectory>) Create();

public:
  static MIKTEXCORECEEAPI(std::unique_ptr<TemporaryDirectory>) Create(const PathName & path);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
