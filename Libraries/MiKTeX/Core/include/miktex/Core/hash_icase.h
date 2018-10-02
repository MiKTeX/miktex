/* miktex/Core/hash_icase:                              -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#if !defined(BE4560F0052846B09E80BD56810BD934)
#define BE4560F0052846B09E80BD56810BD934

#include <miktex/Core/config.h>

#include <cstddef>

#include <string>

#include "Debug.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

struct hash_icase
{
public:
  std::size_t operator() (const std::string& str) const
  {
    // see http://www.isthe.com/chongo/tech/comp/fnv/index.html
#if defined(_M_AMD64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    MIKTEX_ASSERT(sizeof(std::size_t) == 8);
    const std::size_t FNV_prime = 1099511628211ULL;
    const std::size_t offset_basis = 14695981039346656037ULL;
#else
    MIKTEX_ASSERT(sizeof(std::size_t) == 4);
    const std::size_t FNV_prime = 16777619;
    const std::size_t offset_basis = 2166136261;
#endif
    std::size_t hash = offset_basis;
    for (char ch : str)
    {
      if (static_cast<unsigned>(ch) >= 128)
      {
        // ignore UTF-8 chars
        continue;
      }
      else if (ch >= 'a' && ch <= 'z')
      {
        ch = 'A' + (ch - 'a');
      }
      hash ^= (std::size_t)ch;
      hash *= FNV_prime;
    }
    return hash;
  }
};

MIKTEX_CORE_END_NAMESPACE;

#endif
