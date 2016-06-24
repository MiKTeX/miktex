/* miktex/Core/FileStream.h:                            -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(D6DD4259AB704AE6ABDE141135E7C19E)
#define D6DD4259AB704AE6ABDE141135E7C19E

#include <miktex/Core/config.h>

#include <cstddef>
#include <cstdio>

#include "Stream.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// FileStream class. Encapsulates a stdio file stream pointer (FILE*).
class MIKTEXCORETYPEAPI(FileStream) : public Stream
{
public:
  FileStream()
  {
  }

public:
  FileStream(FILE * pFile) :
    pFile(pFile)
  {
  }

public:
  virtual MIKTEXCOREEXPORT MIKTEXTHISCALL ~FileStream();

public:
  MIKTEXCORETHISAPI(void) Attach(FILE * pFile);

  /// Closes the encapsulated stream (except for stdin/stdout/stderr).
public:
  MIKTEXCORETHISAPI(void) Close();

public:
  MIKTEXCORETHISAPI(std::size_t) Read(void * pBytes, std::size_t count) override;

public:
  MIKTEXCORETHISAPI(void) Write(const void * pBytes, std::size_t count) override;

public:
  MIKTEXCORETHISAPI(void) Seek(long offset, SeekOrigin origin) override;

public:
  MIKTEXCORETHISAPI(long) GetPosition() const override;

public:
  MIKTEXCORETHISAPI(void) SetBinary();

public:
  FILE * Get() const
  {
    return pFile;
  }

public:
  FILE * Detach()
  {
    FILE * ret = pFile;
    pFile = nullptr;
    return ret;
  }

private:
  FILE * pFile = nullptr;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
