/* unxMemoryMappedFile.cpp:

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

#if !defined(HAVE_MMAP) && defined(__CYGWIN__)
#  define HAVE_MMAP
#endif

#if !defined(HAVE_MMAP)
#  error this system does not provide the mmap() function
#endif

#include "unxMemoryMappedFile.h"

using namespace MiKTeX::Core;
using namespace std;

MemoryMappedFile * MemoryMappedFile::Create()
{
  return new unxMemoryMappedFile;
}

unxMemoryMappedFile::~unxMemoryMappedFile()
{
  try
  {
    DestroyMapping();
    CloseFile();
  }
  catch (const exception &)
  {
  }
}

void * unxMemoryMappedFile::Open(const char * lpszPath, bool readWrite)
{
  path = lpszPath;
  this->readWrite = readWrite;
  OpenFile();
  CreateMapping(0);
  return ptr;
}

void unxMemoryMappedFile::Close()
{
  DestroyMapping();
  CloseFile();
}

void * unxMemoryMappedFile::Resize(size_t newSize)
{
  DestroyMapping();
  if (ftruncate(filedes, newSize) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftruncate", "path", path);
  }
  CreateMapping(newSize);
  return ptr;
}

void unxMemoryMappedFile::OpenFile()
{
  int oflag(readWrite ? O_RDWR : O_RDONLY);
  filedes = open(path.c_str(), oflag);
  if (filedes < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("open", "path", path);
  }
}

void unxMemoryMappedFile::CreateMapping(size_t maximumFileSize)
{
  struct stat statbuf;

  if (fstat(filedes, &statbuf) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fstat", "path", path);
  }

  if (maximumFileSize == 0)
  {
    maximumFileSize = statbuf.st_size;
  }

  size = maximumFileSize;

  ptr = mmap(nullptr, size, (readWrite ? (PROT_READ | PROT_WRITE) : PROT_READ), MAP_SHARED, filedes, 0);

  if (ptr == MAP_FAILED)
  {
    MIKTEX_FATAL_CRT_ERROR_2("mmap", "path", path);
  }
}

void unxMemoryMappedFile::CloseFile()
{
  if (this->filedes < 0)
  {
    return;
  }
  int filedes = this->filedes;
  this->filedes = -1;
  if (close(filedes) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("close", "path", path);
  }
}

void unxMemoryMappedFile::DestroyMapping()
{
  if (ptr == nullptr)
  {
    return;
  }
  void * ptr = this->ptr;
  this->ptr = nullptr;
  if (munmap(ptr, size) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("munmap", "path", path);
  }
}

void unxMemoryMappedFile::Flush()
{
  if (msync(ptr, size, MS_SYNC) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("msync", "path", path);
  }
}
