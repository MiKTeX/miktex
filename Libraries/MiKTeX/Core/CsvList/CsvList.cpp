/* CsvList.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CsvList.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

class CsvList::impl
{
public:
  CharBuffer<char> buf;
public:
  const char* current = nullptr;
public:
  char* next = nullptr;
public:
  char separator;
};

CsvList::CsvList(const string& s, char separator) :
  pimpl(new impl{})
{
  pimpl->buf = s;
  if (s.empty())
  {
    pimpl->current = nullptr;
    pimpl->next = nullptr;
  }
  else
  {
    pimpl->separator = separator;
    pimpl->current = pimpl->buf.GetData();
    pimpl->next = strchr(pimpl->buf.GetData(), separator);
    if (pimpl->next != nullptr)
    {
      *pimpl->next++ = 0;
    }
  }
}

CsvList::operator bool() const
{
  return pimpl->current != nullptr && pimpl->current[0] != 0;
}

string CsvList::operator*() const
{
  if (pimpl->current == nullptr)
  {
    // TODO: throw
  }
  return pimpl->current;
}

CsvList& CsvList::operator++()
{
  if (pimpl->next != nullptr && *pimpl->next != 0)
  {
    pimpl->current = pimpl->next;
    pimpl->next = strchr(pimpl->next, pimpl->separator);
    if (pimpl->next != nullptr)
    {
      *pimpl->next++ = 0;
    }
  }
  else
  {
    pimpl->current = nullptr;
  }
  return *this;
}

CsvList::~CsvList() noexcept
{
}
