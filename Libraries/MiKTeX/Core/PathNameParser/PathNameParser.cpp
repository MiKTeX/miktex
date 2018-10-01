/* PathNameParser.cpp: path name parser

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "internal.h"

#include "miktex/Core/PathNameParser.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

enum class PathNameParserState
{
  Start,
  Root,
  Path,
  End
};

class PathNameParser::impl
{
public:
  PathName path;
public:
  string current;
public:
  size_t pos = 0;
public:
  PathNameParserState state = PathNameParserState::Start;
};

PathNameParser::PathNameParser(const PathName& path) :
  pimpl(new impl{})
{
  pimpl->path = path;
  ++(*this);
}

PathNameParser::operator bool() const
{
  return !pimpl->current.empty();
}

string PathNameParser::operator*() const
{
  return pimpl->current;
}

PathNameParser& PathNameParser::operator++()
{
  if (pimpl->state == PathNameParserState::Start && IsDirectoryDelimiter(pimpl->path[0]))
  {
    pimpl->current = pimpl->path[0];
    ++pimpl->pos;
    if (IsDirectoryDelimiter(pimpl->path[1]))
    {
      pimpl->state = PathNameParserState::Root;
      pimpl->current += pimpl->path[1];
      ++pimpl->pos;
      for (; pimpl->path[pimpl->pos] != 0 && !IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
      {
        pimpl->current += pimpl->path[pimpl->pos];
      }
    }
    else
    {
      pimpl->state = PathNameParserState::Path;
    }
  }
#if defined(MIKTEX_WINDOWS)
  else if (pimpl->state == PathNameParserState::Start && IsDriveLetter(pimpl->path[0]) && pimpl->path[1] == ':')
  {
    pimpl->state = PathNameParserState::Root;
    pimpl->current = pimpl->path[0];
    ++pimpl->pos;
    pimpl->current += pimpl->path[1];
    ++pimpl->pos;
  }
#endif
  else if (pimpl->state == PathNameParserState::Root)
  {
    MIKTEX_ASSERT(IsDirectoryDelimiter(pimpl->path[pimpl->pos]));
    pimpl->current = pimpl->path[pimpl->pos];
    pimpl->state = PathNameParserState::Path;
  }
  else
  {
    for (; pimpl->path[pimpl->pos] != 0 && PathName::IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
    {
    }
    pimpl->current = "";
    for (; pimpl->path[pimpl->pos] != 0 && !PathName::IsDirectoryDelimiter(pimpl->path[pimpl->pos]); ++pimpl->pos)
    {
      pimpl->current += pimpl->path[pimpl->pos];
    }
  }
  return *this;
}

PathNameParser::~PathNameParser() noexcept
{
}
