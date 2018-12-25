/* winStringUtil.cpp:

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

#include <Windows.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/CharBuffer.h"
#include "miktex/Util/StringUtil.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

MIKTEXSTATICFUNC(wchar_t*) AnsiToWideChar(const char* source, wchar_t* dest, size_t destSize)
{
  if (*source == 0)
  {
    // TODO: MIKTEX_ASSERT(destSize > 0);
    *dest = 0;
    return dest;
  }
  int n = MultiByteToWideChar(CP_ACP, 0, source, -1, dest, static_cast<int>(destSize));
  if (n == 0)
  {
    throw exception("Conversion from narrow string (ANSI) to wide character string did not succeed.");
  }
  if (n < 0)
  {
    FATAL_ERROR();
  }
  return dest;
}

string StringUtil::AnsiToUTF8(const char* ansi)
{
  CharBuffer<wchar_t, 512> buf(strlen(ansi) + 1);
  return StringUtil::WideCharToUTF8(AnsiToWideChar(ansi, buf.GetData(), buf.GetCapacity()));
}
