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

Tokenizer::Tokenizer(const char * lpsz, const char * lpszDelim) :
  Base(lpsz)
{
  lpszNext = GetData();
  lpszCurrent = nullptr;
  pDelims = nullptr;
  SetDelim(lpszDelim);
  FindToken();
}

void Tokenizer::SetDelim(const char * lpszDelim)
{
  bitset<256> * psetDelim = reinterpret_cast<bitset<256>*>(pDelims);
  if (psetDelim == nullptr)
  {
    psetDelim = new bitset<256>;
    pDelims = psetDelim;
  }
  psetDelim->reset();
  // TODO: MIKTEX_ASSERT_STRING(lpszDelim);
  for (; *lpszDelim != 0; ++lpszDelim)
  {
    psetDelim->set(static_cast<unsigned char>(*lpszDelim));
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
    if (pDelims != nullptr)
    {
      bitset<256> * pDelims = reinterpret_cast<bitset<256>*>(this->pDelims);
      this->pDelims = nullptr;
      delete pDelims;
    }
    lpszCurrent = nullptr;
    lpszNext = nullptr;
  }
  catch (const exception &)
  {
  }
}

void Tokenizer::FindToken()
{
  // TODO: MIKTEX_ASSERT(lpszNext != nullptr);
  lpszCurrent = lpszNext;
  bitset<256> * pDelims = reinterpret_cast<bitset<256>*>(this->pDelims);
  // TODO: MIKTEX_ASSERT(pDelims != nullptr);
  while ((*pDelims)[static_cast<unsigned char>(*lpszCurrent)] && *lpszCurrent != 0)
  {
    ++lpszCurrent;
  }
  for (lpszNext = const_cast<char*>(lpszCurrent); *lpszNext != 0; ++lpszNext)
  {
    if ((*pDelims)[static_cast<unsigned char>(*lpszNext)])
    {
      *lpszNext = 0;
      ++lpszNext;
      break;
    }
  }
}
