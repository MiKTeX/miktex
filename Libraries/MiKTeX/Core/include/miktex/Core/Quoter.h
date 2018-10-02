/* miktex/Core/Quoter.h:                                -*- C++ -*-

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

#if !defined(CEFA448433DD489882E2BFB307C48DD9)
#define CEFA448433DD489882E2BFB307C48DD9

#include <miktex/Core/config.h>

#include <string>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/inliners.h>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

template<typename CharType> class Quoter :
  public MiKTeX::Util::CharBuffer<CharType>
{
public:
  Quoter() = delete;

public:
  Quoter(const Quoter& other) = delete;

public:
  Quoter& operator=(const Quoter& other) = delete;

public:
  Quoter(Quoter&& other) = delete;

public:
  Quoter& operator= (Quoter&& other) = delete;

public:
  ~Quoter() = default;

public:
  Quoter(const CharType* lpsz)
  {
    Quote(lpsz);
  }

public:
  Quoter(const std::basic_string<CharType>& s)
  {
    Quote(s.c_str());
  }

public:
  Quoter(const PathName& path)
  {
    Quote(path.GetData());
  }

private:
  void Quote(const CharType* lpsz)
  {
    bool needQuotes = (*lpsz == 0 || MiKTeX::Util::StrChr(lpsz, 0x20/*' '*/) != nullptr);
    if (needQuotes)
    {
      MiKTeX::Util::CharBuffer<CharType>::Append(0x22/*'"'*/);
    }
    MiKTeX::Util::CharBuffer<CharType>::Append(lpsz);
    if (needQuotes)
    {
      MiKTeX::Util::CharBuffer<CharType>::Append(0x22/*'"'*/);
    }
  }
};

MIKTEX_CORE_END_NAMESPACE;

#endif
