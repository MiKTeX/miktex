/* miktex/Core/Stream.h:                                -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if !defined(A5C8AD7AABA6476FA2A563F6C670BC2F)
#define A5C8AD7AABA6476FA2A563F6C670BC2F

#include <miktex/Core/config.h>

#include <cstddef>

MIKTEX_CORE_BEGIN_NAMESPACE;

enum class SeekOrigin
{
  Begin,
  Current,
  End,
};

class MIKTEXNOVTABLE Stream
{
public:
  virtual MIKTEXTHISCALL ~Stream() = 0;

public:
  virtual std::size_t MIKTEXTHISCALL Read(void * pBytes, std::size_t count) = 0;

public:
  virtual void MIKTEXTHISCALL Write(const void * pBytes, std::size_t count) = 0;

public:
  virtual void MIKTEXTHISCALL Seek(long offset, SeekOrigin origin) = 0;

public:
  virtual long MIKTEXTHISCALL GetPosition() const = 0;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
