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

#include "config.h"

#include <fcntl.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <io.h>
#endif

#include <miktex/Core/FileStream>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;

FileStream::~FileStream() noexcept
{
  try
  {
    Close();
  }
  catch (const exception&)
  {
  }
}

void FileStream::Attach(FILE* file)
{
  if (this->file != nullptr)
  {
    Close();
  }
  this->file = file;
}

void FileStream::Close()
{
  if (file != nullptr)
  {
    FILE* file_ = file;
    file = nullptr;
    if (file_ != stdin && file_ != stdout && file_ != stderr)
    {
      if (fclose(file_) != 0)
      {
	MIKTEX_FATAL_CRT_ERROR("fclose");
      }
    }
  }
}

size_t FileStream::Read(void* data, size_t count)
{
  size_t n = fread(data, 1, count, file);
  if (ferror(file) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fread");
  }
  return n;
}

void FileStream::Write(const void* data, size_t count)
{
  if (fwrite(data, 1, count, file) != count)
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
  if (fseek(file, offset, origin) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fseek");
  }
}

long FileStream::GetPosition() const
{
  long pos = ftell(file);
  if (pos < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("ftell");
  }
  return pos;
}

void FileStream::SetBinary()
{
#if defined(MIKTEX_WINDOWS)
  if (_setmode(_fileno(file), _O_BINARY) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_setmode");
  }
#endif
}
