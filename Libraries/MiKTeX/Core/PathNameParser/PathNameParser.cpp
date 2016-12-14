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

#include "miktex/Core/PathNameParser.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

class PathNameParser::impl
{
public:
  PathName path;
public:
  char * current = nullptr;
public:
  char * next = nullptr;
};

PathNameParser::PathNameParser(const PathName & path) :
  pimpl(new impl{})
{
  pimpl->path = path;
  pimpl->next = pimpl->path.GetData();
  pimpl->current = nullptr;
  ++(*this);
}

PathNameParser::operator bool() const
{
  return pimpl->current != nullptr && pimpl->current[0] != 0;
}

string PathNameParser::operator*() const
{
  if (pimpl->current == nullptr)
  {
    // TODO: throw
  }
  return pimpl->current;
}

PathNameParser & PathNameParser::operator++ ()
{
  pimpl->current = pimpl->next;

  char * lpsz;

  if (pimpl->current == pimpl->path.GetData() && IsDirectoryDelimiter(pimpl->path[0]))
  {
    if (IsDirectoryDelimiter(pimpl->path[1]))
    {
      lpsz = &pimpl->path[2];
    }
    else
    {
      lpsz = &pimpl->path[1];
    }
  }
#if defined(MIKTEX_WINDOWS)
  else if (pimpl->current == pimpl->path.GetData() && IsDriveLetter(pimpl->path[0]) && pimpl->path[1] == ':' && IsDirectoryDelimiter(pimpl->path[2]))
  {
    lpsz = &pimpl->path[3];
  }
#endif
  else
  {
    // skip extra directory delimiters
    for (; PathName::IsDirectoryDelimiter(*pimpl->current); ++pimpl->current)
    {
      ;
    }
    lpsz = pimpl->current;
  }

  // cut out the next component
  for (pimpl->next = lpsz; *pimpl->next != 0; ++pimpl->next)
  {
    if (PathName::IsDirectoryDelimiter(*pimpl->next))
    {
      *pimpl->next = 0;
      ++pimpl->next;
      break;
    }
  }

  return *this;
}


PathNameParser::~PathNameParser()
{
}
