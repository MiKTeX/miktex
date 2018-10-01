/* miktex/Core/equal_icase_dos:                         -*- C++ -*-

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

#pragma once

#if !defined(B9BC6F86436E4103A4836E2C16CA8343)
#define B9BC6F86436E4103A4836E2C16CA8343

#include <miktex/Core/config.h>

#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

struct less_icase_dos
{
public:
  bool operator() (const std::string & str1, const std::string & str2) const
  {
    std::size_t l1 = str1.length();
    std::size_t l2 = str2.length();
    std::size_t count = l1 < l2 ? l1 : l2;
    for (std::size_t idx = 0; idx < count; ++idx)
    {
      char ch1 = normalize(str1[idx]);
      char ch2 = normalize(str2[idx]);
      if (ch1 != ch2)
      {
        return ch1 < ch2;
      }
    }
    return l1 < l2;
  }
private:
  char normalize(char ch) const
  {
    if (ch == '/')
    {
      return '\\';
    }
    else if (ch >= 'A' && ch <= 'Z')
    {
      return ch - 'A' + 'a';
    }
    else
    {
      return ch;
    }
  }
};

MIKTEX_CORE_END_NAMESPACE;

#endif
