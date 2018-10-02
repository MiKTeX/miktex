/* miktex/Core/HasNamedValues.h:                        -*- C++ -*-

   Copyright (C) 2016-2018 Christian Schenk

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

#if !defined(A7418B137F044A99E8FA03A53FC5524A)
#define A7418B137F044A99E8FA03A53FC5524A

#include <miktex/Core/config.h>

#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE HasNamedValues
{
public:
  virtual bool MIKTEXTHISCALL TryGetValue(const std::string& valueName, std::string& value) = 0;
  virtual std::string MIKTEXTHISCALL GetValue(const std::string& valueName) = 0;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
