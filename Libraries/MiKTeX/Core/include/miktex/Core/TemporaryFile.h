/* miktex/Core/TemporaryFile.h:                         -*- C++ -*-

   Copyright (C) 2016-2019 Christian Schenk

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

#if !defined(C0A8542261034139A6B6614B20D5EC8F)
#define C0A8542261034139A6B6614B20D5EC8F

#include <miktex/Core/config.h>

#include <memory>

#include <miktex/Util/PathName>

MIKTEX_CORE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE TemporaryFile
{
public:
  virtual MIKTEXTHISCALL ~TemporaryFile() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL Delete() = 0;

public:
  virtual MiKTeX::Util::PathName MIKTEXTHISCALL GetPathName() const = 0;

public:
  virtual void MIKTEXTHISCALL Keep() = 0;

public:
  static MIKTEXCORECEEAPI(std::unique_ptr<TemporaryFile>) Create();

public:
  static MIKTEXCORECEEAPI(std::unique_ptr<TemporaryFile>) Create(const MiKTeX::Util::PathName& path);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
