/* miktex/Core/LibraryVersion.h:                        -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

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

#if !defined(AB1AD5AD3B884FC5B7A00F69767B3375)
#define AB1AD5AD3B884FC5B7A00F69767B3375

#include <miktex/Core/config.h>

#include <string>

#include "VersionNumber.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

struct LibraryVersion
{
  LibraryVersion(const std::string& key, const std::string& fromHeader, const std::string& fromRuntime) :
    key(key),
    name(key),
    fromHeader(fromHeader),
    fromRuntime(fromRuntime)
  {
  }
  LibraryVersion(const std::string& key, const std::string& name, const std::string& fromHeader, const std::string& fromRuntime) :
    key(key),
    name(name),
    fromHeader(fromHeader),
    fromRuntime(fromRuntime)
  {
  }
  LibraryVersion(const std::string& key, const MiKTeX::Core::VersionNumber* fromHeader, const MiKTeX::Core::VersionNumber* fromRuntime) :
    key(key),
    name(key),
    fromHeader(fromHeader == nullptr ? std::string() : fromHeader->ToString()),
    fromRuntime(fromRuntime == nullptr ? std::string() : fromRuntime->ToString())
  {
  }
  std::string key;
  std::string name;
  std::string description;
  std::string fromHeader;
  std::string fromRuntime;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
