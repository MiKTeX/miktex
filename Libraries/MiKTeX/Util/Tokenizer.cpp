/* Tokenizer.cpp: splitting strings

   Copyright (C) 1996-2018 Christian Schenk

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

#include "internal.h"

#include "miktex/Util/CharBuffer.h"

class Tokenizer::impl
{
public:
  CharBuffer<char> buf;
public:
  const char* current = nullptr;
public:
  char* next = nullptr;
public:
  bitset<256> delims;
};

Tokenizer::Tokenizer(const string& s, const string& delims) :
  pimpl(new impl{})
{
  pimpl->buf = s;
  pimpl->next = pimpl->buf.GetData();
  SetDelimiters(delims);
  this->operator++();
}

void Tokenizer::SetDelimiters(const string& delims)
{
  pimpl->delims.reset();
  for (const char& ch : delims)
  {
    pimpl->delims.set(static_cast<unsigned char>(ch));
  }
}

Tokenizer::operator bool() const
{
  return pimpl->current != nullptr && pimpl->current[0] != 0;
}

string Tokenizer::operator*() const
{
  if (pimpl->current == nullptr)
  {
    // TODO: throw
  }
  return pimpl->current;
}

Tokenizer& Tokenizer::operator++()
{
  // TODO: MIKTEX_ASSERT(pimpl->next != nullptr);
  pimpl->current = pimpl->next;
  while (pimpl->delims[static_cast<unsigned char>(*pimpl->current)] && *pimpl->current != 0)
  {
    ++pimpl->current;
  }
  for (pimpl->next = const_cast<char*>(pimpl->current); *pimpl->next != 0; ++pimpl->next)
  {
    if (pimpl->delims[static_cast<unsigned char>(*pimpl->next)])
    {
      *pimpl->next = 0;
      ++pimpl->next;
      break;
    }
  }
  return *this;
}

Tokenizer::~Tokenizer() noexcept
{
}
