/* miktex/Configuration/HasNamedValues.h:

   Copyright (C) 2016-2021 Christian Schenk

   This file is part of the MiKTeX Configuration Library.

   The MiKTeX Configuration Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.
   
   The MiKTeX Configuration Library is distributed in the hope that it
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Configuration Library; if not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include <miktex/Configuration/config.h>

#include <string>

MIKTEX_CONFIG_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE HasNamedValues
{
public:
  virtual bool MIKTEXTHISCALL TryGetValue(const std::string& valueName, std::string& value) = 0;
  virtual std::string MIKTEXTHISCALL GetValue(const std::string& valueName) = 0;
};

MIKTEX_CONFIG_END_NAMESPACE;
