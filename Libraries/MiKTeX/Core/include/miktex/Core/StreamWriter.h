/* miktex/Core/StreamWriter.h:                          -*- C++ -*-

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

#if !defined(ACBE4B57EA604A0289F0EED42F80F8B1)
#define ACBE4B57EA604A0289F0EED42F80F8B1

#include <miktex/Core/config.h>

#include <string>

#include "FileStream.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class StreamWriter
{
public:
  StreamWriter() = default;

public:
  StreamWriter(const StreamWriter& other) = delete;

public:
  StreamWriter operator=(const StreamWriter& other) = delete;

public:
  StreamWriter(StreamWriter&& other) = delete;

public:
  StreamWriter operator=(StreamWriter&& other) = delete;

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~StreamWriter() noexcept;

public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL StreamWriter(const PathName& path);

public:
  bool IsOpen() const
  {
    return stream.GetFile() != nullptr;
  }

public:
  void Flush()
  {
    fflush(stream.GetFile());
  }

public:
  MIKTEXCORETHISAPI(void) Write(char ch);

public:
  MIKTEXCORETHISAPI(void) Write(const std::string& line);

public:
  MIKTEXCORETHISAPI(void) WriteLine(const std::string& line);

public:
  MIKTEXCORETHISAPI(void) WriteLine();

public:
  MIKTEXCORETHISAPI(void) Close();

private:
  FileStream stream;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
