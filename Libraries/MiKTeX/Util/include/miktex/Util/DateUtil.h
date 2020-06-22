/* miktex/Util/DateUtil.h:

   Copyright (C) 2020 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "config.h"

#include <ctime>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE DateUtil
{
public:
  DateUtil() = delete;

public:
  DateUtil(const DateUtil& other) = delete;

public:
  DateUtil& operator=(const DateUtil& other) = delete;

public:
  DateUtil(DateUtil&& other) = delete;

public:
  DateUtil& operator=(DateUtil&& other) = delete;

public:
  ~DateUtil() = delete;

public:
  static constexpr std::time_t UNDEFINED_TIME_T_VALUE = static_cast<std::time_t>(0);

public:
  static bool IsDefined(std::time_t time)
  {
    return time != static_cast<std::time_t>(0) && time != static_cast<std::time_t>(-1);
  }
};

MIKTEX_UTIL_END_NAMESPACE;
