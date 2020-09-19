/* miktex/Core/PathNameParser.h:                        -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#if !defined(E46FF47278D7421D9657C798FB641B8C)
#define E46FF47278D7421D9657C798FB641B8C

#include <miktex/Core/config.h>

#include <memory>
#include <string>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class PathNameParser
{
public:
  PathNameParser() = delete;

public:
  PathNameParser(const PathNameParser& other) = delete;

public:
  PathNameParser& operator=(const PathNameParser& other) = delete;

public:
  PathNameParser(PathNameParser&& other) = delete;

public:
  PathNameParser& operator=(PathNameParser&& other) = delete;

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~PathNameParser() noexcept;

public:
  explicit MIKTEXCOREEXPORT MIKTEXTHISCALL PathNameParser(const PathName& path);

public:
  explicit MIKTEXCOREEXPORT MIKTEXTHISCALL operator bool() const;

public:
  MIKTEXCORETHISAPI(std::string) operator*() const;

public:
  MIKTEXCORETHISAPI(PathNameParser&) operator++();

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
