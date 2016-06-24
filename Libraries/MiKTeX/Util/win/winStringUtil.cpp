/* winStringUtil.cpp:

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

MIKTEXSTATICFUNC(wchar_t *) AnsiToWideChar(const char * lpszAnsi, wchar_t * lpszWideChar, size_t size)
{
  if (*lpszAnsi == 0)
  {
    *lpszWideChar = 0;
    return lpszWideChar;
  }
  int n = MultiByteToWideChar(CP_ACP, 0, lpszAnsi, -1, lpszWideChar, size);
  if (n == 0)
  {
    throw exception("Conversion from narrow string (ANSI) to wide character string did not succeed.");
  }
  if (n < 0)
  {
    FATAL_ERROR();
  }
  return lpszWideChar;
}

string StringUtil::AnsiToUTF8(const char * lpszAnsi)
{
  CharBuffer<wchar_t, 512> buf(strlen(lpszAnsi) + 1);
  return StringUtil::WideCharToUTF8(AnsiToWideChar(lpszAnsi, buf.GetData(), buf.GetCapacity()));
}
