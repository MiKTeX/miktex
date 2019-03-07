/* miktex/PackageManager/definitions.h:                 -*- C++ -*-

   Copyright (C) 2001-2019 Christian Schenk

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

#pragma once

#if !defined(C09DF2DF4A8A43C5B815850EFF9BEED9)
#define C09DF2DF4A8A43C5B815850EFF9BEED9

MIKTEX_PACKAGES_BEGIN_NAMESPACE;

/// An invalid timestamp.
constexpr std::time_t InvalidTimeT = static_cast<std::time_t>(0);

/// Check a timestamp.
/// @param time The timestamp to check.
/// @return Returns `true`, if this is a valid timestamp.
inline bool IsValidTimeT(std::time_t time)
{
  return time != static_cast<std::time_t>(0) && time != static_cast<std::time_t>(-1);
}

MIKTEX_PACKAGES_END_NAMESPACE;

#endif
