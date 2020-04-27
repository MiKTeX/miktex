/* miktex/Util/Tokenizer.h:                             -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#if !defined(B8F781CD4C5C48639A1A9A90A45F9855)
#define B8F781CD4C5C48639A1A9A90A45F9855

#include "config.h"

#include <memory>
#include <string>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class Tokenizer
{
public:
  Tokenizer() = delete;

public:
  Tokenizer(const Tokenizer& other) = delete;

public:
  Tokenizer& operator=(const Tokenizer& other) = delete;

public:
  Tokenizer(Tokenizer&& other) = delete;

public:
  Tokenizer& operator=(Tokenizer&& other) = delete;

public:
  virtual MIKTEXUTILEXPORT MIKTEXTHISCALL ~Tokenizer() noexcept;

public:
  MIKTEXUTILEXPORT MIKTEXTHISCALL Tokenizer(const std::string& s, const std::string& delims);

public:
  MIKTEXUTILTHISAPI(void) SetDelimiters(const std::string& delims);

public:
  explicit MIKTEXUTILEXPORT MIKTEXTHISCALL operator bool() const;

public:
  MIKTEXUTILTHISAPI(std::string) operator*() const;

public:
  MIKTEXUTILTHISAPI(Tokenizer&) operator++();

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEX_UTIL_END_NAMESPACE;

#endif
