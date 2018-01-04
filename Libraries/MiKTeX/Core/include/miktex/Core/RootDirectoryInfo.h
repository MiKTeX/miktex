/* RootDirectoryInfo.h:                                 -*- C++ -*-

   Copyright (C) 2018 Christian Schenk

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

#if !defined(F880DB7F8908430C9C86844870CC44EE)
#define F880DB7F8908430C9C86844870CC44EE

#include <miktex/Core/config.h>

#include "OptionSet.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

struct RootDirectoryInfo
{
public:
  enum class Purpose
  {
    Generic,
    Config,
    Data,
    Install
  };

public:
  typedef MiKTeX::Core::OptionSet<Purpose> Purposes;

public:
  enum class Attribute
  {
    Common,
    Other
  };

public:
  typedef MiKTeX::Core::OptionSet<Attribute> Attributes;


  // fully qualified path to root directory
public:
  MiKTeX::Core::PathName path;

public:
  Purposes purposes;

public:
  Attributes attributes;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
