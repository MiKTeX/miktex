/* FileStream.cpp: file stream

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

#include <fcntl.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <io.h>
#endif

#include "internal.h"

#include "miktex/Core/FileStream.h"

using namespace MiKTeX::Core;
using namespace std;

FileStream::~FileStream() noexcept
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void FileStream::Attach(FILE * pFile)
{
  if (this->pFile != nullptr)
  {
    Close();
  }
  this->pFile = pFile;
}

void FileStream::Close()
{
  if (pFile != nullptr)
  {
    FILE * pFile_ = pFile;
    pFile = nullptr;
    if (pFile_ != stdin && pFile_ != stdout && pFile_ != stderr)
    {
      if (fclose(pFile_) != 0)
      {
	MIKTEX_FATAL_CRT_ERROR("fclose");
      }
    }
  }
}

size_t FileStream::Read(void * pBytes, size_t count)
{
  size_t n = fread(pBytes, 1, count, pFile);
  if (ferror(pFile) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fread");
  }
  return n;
}

void FileStream::Write(const void * pBytes, size_t count)
{
  if (fwrite(pBytes, 1, count, pFile) != count)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
}

void FileStream::Seek(long offset, SeekOrigin seekOrigin)
{
  int origin;
  switch (seekOrigin)
  {
  case SeekOrigin::Begin:
    origin = SEEK_SET;
    break;
  case SeekOrigin::End:
    origin = SEEK_END;
    break;
  case SeekOrigin::Current:
    origin = SEEK_CUR;
    break;
  default:
    MIKTEX_UNEXPECTED();
    break;
  }
  if (fseek(pFile, offset, origin) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fseek");
  }
}

long FileStream::GetPosition() const
{
  long pos = ftell(pFile);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("ftell");
  }
  return pos;
}

void FileStream::SetBinary()
{
#if defined(MIKTEX_WINDOWS)
  if (_setmode(_fileno(pFile), _O_BINARY) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_setmode");
  }
#endif
}
