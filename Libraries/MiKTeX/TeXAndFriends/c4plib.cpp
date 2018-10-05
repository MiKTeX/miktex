/* c4plib.cpp: C4P runtime routines

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#define C4P_BEGIN_NAMESPACE namespace C4P {
#define C4P_END_NAMESPACE }

C4P_BEGIN_NAMESPACE;

C4PCEEAPI(void) DiscardLine(C4P_text& textfile)
{
  MIKTEX_API_BEGIN("DiscardLine");
  textfile.AssertValid();
  // FIXME: OS X
  while (!feof(textfile) && GetChar(textfile) != '\n')
  {
    ;
  }
  MIKTEX_API_END("DiscardLine");
}

C4PCEEAPI(char) GetChar(C4P_text& textfile)
{
  MIKTEX_API_BEGIN("GetChar");
  textfile.AssertValid();
  char ch = *textfile;
  int ch2 = GetC(textfile);
  if (ch2 != EOF)
  {
    *textfile = static_cast<char>(ch2);
  }
  return ch;
  MIKTEX_API_END("GetChar");
}

C4PCEEAPI(C4P_integer) GetInteger(C4P_text& textfile)
{
  MIKTEX_API_BEGIN("GetInteger");
  textfile.AssertValid();
  int ch = GetChar(textfile);
  int sign = (ch == '-' ? -1 : 1);
  C4P_integer result = 0;
  if (ch == '+' || ch == '-')
  {
    ch = GetChar(textfile);
  }
  while (isdigit(ch))
  {
    result *= 10;
    result += (ch - '0');
    ch = GetChar(textfile);
  }
  return result * sign;
  MIKTEX_API_END("GetInteger");
}

C4PCEEAPI(C4P_real) GetReal(C4P_text& /*textfile*/)
{
  MIKTEX_API_BEGIN("GetReal");
  MIKTEX_UNEXPECTED();
  MIKTEX_API_END("GetReal");
}

bool FileRoot::Open(const PathName& path, FileMode mode, FileAccess access, FileShare share, bool text, bool mustExist)
{
  MIKTEX_API_BEGIN("FileRoot::open");
  FILE* pFile;
  shared_ptr<Session> session = Session::Get();
  if (mustExist)
  {
    pFile = session->OpenFile(path, mode, access, text, share);
  }
  else
  {
    pFile = session->TryOpenFile(path, mode, access, text, share);
    if (pFile == nullptr)
    {
      return false;
    }
  }
  Attach(pFile, true);
  return true;
  MIKTEX_API_END("FileRoot::open");
}

C4PCEEAPI(C4P_integer) Round(double r)
{
  MIKTEX_API_BEGIN("Round");
  if (r > INT_MAX)
  {
    return INT_MAX;
  }
  else if (r < -INT_MAX)
  {
    return -INT_MAX;
  }
  else if (r >= 0.0)
  {
    return static_cast<C4P_integer>(r + 0.5);
  }
  else
  {
    return static_cast<C4P_integer>(r - 0.5);
  }
  MIKTEX_API_END("Round");
}

C4P_END_NAMESPACE;
