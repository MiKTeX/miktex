/* PathNameParser.cpp: path name parser

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/BufferSizes.h"
#include "miktex/Core/PathNameParser.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

PathNameParser::PathNameParser(const char * lpszPath)
{
  StringUtil::CopyString(buffer, BufferSizes::MaxPath, lpszPath);
  lpszNext = buffer;
  lpszCurrent = nullptr;
  ++(*this);
}

const char * PathNameParser::operator++ ()
{
  lpszCurrent = lpszNext;

  char * lpsz;

  if (lpszCurrent == buffer && IsDirectoryDelimiter(buffer[0]))
  {
    if (IsDirectoryDelimiter(buffer[1]))
    {
      lpsz = &buffer[2];
    }
    else
    {
      lpsz = &buffer[1];
    }
  }
#if defined(MIKTEX_WINDOWS)
  else if (lpszCurrent == buffer && IsDriveLetter(buffer[0]) && buffer[1] == ':' && IsDirectoryDelimiter(buffer[2]))
  {
    lpsz = &buffer[3];
  }
#endif
  else
  {
    // skip extra directory delimiters
    for (; PathName::IsDirectoryDelimiter(*lpszCurrent); ++lpszCurrent)
    {
      ;
    }
    lpsz = lpszCurrent;
  }

  // cut out the next component
  for (lpszNext = lpsz; *lpszNext != 0; ++lpszNext)
  {
    if (PathName::IsDirectoryDelimiter(*lpszNext))
    {
      *lpszNext = 0;
      ++lpszNext;
      break;
    }
  }

  return GetCurrent();
}
