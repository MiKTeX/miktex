/* miktex/Core/StreamReader.h:                          -*- C++ -*-

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

#pragma once

#if !defined(CE1273C4ECD94AE2ACFFFAB81B913AED)
#define CE1273C4ECD94AE2ACFFFAB81B913AED

#include <miktex/Core/config.h>

#include "FileStream.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class StreamReader
{
public:
  StreamReader() = default;

public:
  StreamReader(const StreamReader& other) = delete;

public:
  StreamReader& operator=(const StreamReader& other) = delete;

public:
  StreamReader(StreamReader&& other) = delete;

public:
  StreamReader& operator=(StreamReader&& other) = delete;

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~StreamReader() noexcept;

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL StreamReader(const PathName& path);

public:
  StreamReader(bool readStdin) :
    stream(readStdin ? stdin : nullptr)
  {
  }

public:
  MIKTEXCORETHISAPI(bool) ReadLine(std::string& line);

public:
  MIKTEXCORETHISAPI(void) Close();

private:
  FileStream stream;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
