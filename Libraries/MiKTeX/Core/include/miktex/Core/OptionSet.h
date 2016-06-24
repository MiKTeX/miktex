/* miktex/Core/OptionSet.h:                             -*- C++ -*-

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

#if !defined(DA160798149E44048A2AD243F813ACCD)
#define DA160798149E44048A2AD243F813ACCD

#include <miktex/Core/config.h>

#include <cstddef>

#include <bitset>
#include <initializer_list>

MIKTEX_CORE_BEGIN_NAMESPACE;

template<typename EnumClass_, int Size_ = 32> class OptionSet
{
public:
  OptionSet()
  {
  }

public:
  OptionSet(const std::initializer_list<EnumClass_> & options)
  {
    for (const EnumClass_ & opt : options)
    {
      bits[(std::size_t)opt] = true;
    }
  }

public:
  OptionSet & operator += (EnumClass_ rhs)
  {
    bits[(std::size_t)rhs] = true;
    return *this;
  }

public:
  OptionSet & operator += (const OptionSet<EnumClass_> & rhs)
  {
    this->bits |= rhs.bits;
    return *this;
  }

public:
  OptionSet & operator -= (EnumClass_ rhs)
  {
    bits[(std::size_t)rhs] = false;
    return *this;
  }

public:
  bool operator[](EnumClass_ opt) const
  {
    return bits[(std::size_t)opt];
  }

public:
  bool operator==(const OptionSet<EnumClass_> & rhs) const
  {
    return bits == rhs.bits;
  }

public:
  bool operator!=(const OptionSet<EnumClass_> & rhs) const
  {
    return bits != rhs.bits;
  }

public:
  void Reset()
  {
    bits.reset();
  }

private:
  std::bitset<Size_> bits;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
