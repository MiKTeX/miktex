/* miktex/Core/CSVList.h:                               -*- C++ -*-

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

#if !defined(FB648E67CF8147BEB6646148F256DC30)
#define FB648E67CF8147BEB6646148F256DC30

#include <miktex/Core/config.h>

#include <string>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/inliners.h>

MIKTEX_CORE_BEGIN_NAMESPACE;

template<typename CharType, int BUFSIZE> class BasicCsvList :
  protected MiKTeX::Util::CharBuffer<CharType, BUFSIZE>
{
public:
  BasicCsvList()
  {
  }

public:
  BasicCsvList(const CharType * lpszValueList, CharType separator) :
    MiKTeX::Util::CharBuffer<CharType, BUFSIZE>(lpszValueList)
  {
    if (lpszValueList == nullptr || *lpszValueList == 0)
    {
      lpszNext = lpszCurrent = nullptr;
    }
    else
    {
      this->separator = separator;
      lpszCurrent = MiKTeX::Util::CharBuffer<CharType, BUFSIZE>::GetData();
      lpszNext = const_cast<CharType*>(MiKTeX::Util::StrChr(MiKTeX::Util::CharBuffer<CharType, BUFSIZE>::GetData(), separator));
      if (lpszNext != nullptr)
      {
        *lpszNext++ = 0;
      }
    }
  }

public:
  BasicCsvList(const std::basic_string<CharType> & valueList, CharType separator) :
    BasicCsvList(valueList.c_str(), separator)
  {
  }

public:
  BasicCsvList(const BasicCsvList & rhs) = delete;

public:
  BasicCsvList & operator= (const BasicCsvList & rhs) = delete;

public:
  const CharType * operator ++ ()
  {
    if (lpszNext != nullptr && *lpszNext != 0)
    {
      lpszCurrent = lpszNext;
      lpszNext = const_cast<CharType*>(MiKTeX::Util::StrChr(lpszNext, separator));
      if (lpszNext != nullptr)
      {
        *lpszNext++ = 0;
      }
    }
    else
    {
      lpszCurrent = nullptr;
    }
    return GetCurrent();
  }

public:
  const CharType * GetCurrent() const
  {
    return lpszCurrent;
  }

private:
  CharType separator;

private:
  CharType * lpszNext = nullptr;

private:
  CharType * lpszCurrent = nullptr;
};

typedef BasicCsvList<char, 512> CSVList;

MIKTEX_CORE_END_NAMESPACE;

#endif
