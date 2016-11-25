/* Tokenizer.cpp: splitting strings

   Copyright (C) 1996-2016 Christian Schenk

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

Tokenizer::Tokenizer(const char * lpsz, const char * delims) :
  Base(lpsz)
{
  next = GetData();
  current = nullptr;
  this->delims = nullptr;
  SetDelim(delims);
  FindToken();
}

void Tokenizer::SetDelim(const char * delims)
{
  bitset<256> * setDelims = reinterpret_cast<bitset<256>*>(this->delims);
  if (setDelims == nullptr)
  {
    setDelims = new bitset<256>;
    this->delims = setDelims;
  }
  setDelims->reset();
  // TODO: MIKTEX_ASSERT_STRING(lpszDelim);
  for (; *delims != 0; ++delims)
  {
    setDelims->set(static_cast<unsigned char>(*delims));
  }
}

const char * Tokenizer::operator++ ()
{
  FindToken();
  return GetCurrent();
}

Tokenizer::~Tokenizer()
{
  try
  {
    if (delims != nullptr)
    {
      bitset<256> * setDelims = reinterpret_cast<bitset<256>*>(delims);
      delims = nullptr;
      delete setDelims;
    }
    current = nullptr;
    next = nullptr;
  }
  catch (const exception &)
  {
  }
}

void Tokenizer::FindToken()
{
  // TODO: MIKTEX_ASSERT(lpszNext != nullptr);
  current = next;
  bitset<256> * setDelims = reinterpret_cast<bitset<256>*>(delims);
  // TODO: MIKTEX_ASSERT(pDelims != nullptr);
  while ((*setDelims)[static_cast<unsigned char>(*current)] && *current != 0)
  {
    ++current;
  }
  for (next = const_cast<char*>(current); *next != 0; ++next)
  {
    if ((*setDelims)[static_cast<unsigned char>(*next)])
    {
      *next = 0;
      ++next;
      break;
    }
  }
}
