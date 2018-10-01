/* StreamReader.cpp:

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

#include "miktex/Core/StreamReader.h"

using namespace MiKTeX::Core;
using namespace std;

StreamReader::StreamReader(const PathName & path) :
  stream(File::Open(path, FileMode::Open, FileAccess::Read))
{
}

StreamReader::~StreamReader() noexcept
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void StreamReader::Close()
{
  stream.Close();
}

bool StreamReader::ReadLine(string & line)
{
  return Utils::ReadLine(line, stream.Get(), false);
}
