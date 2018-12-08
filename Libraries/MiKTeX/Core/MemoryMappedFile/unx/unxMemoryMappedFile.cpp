/* unxMemoryMappedFile.cpp:

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
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "internal.h"

#include "unxMemoryMappedFile.h"

using namespace MiKTeX::Core;
using namespace std;

MemoryMappedFile* MemoryMappedFile::Create()
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
  catch (const exception&)
  {
  }
}

void* unxMemoryMappedFile::Open(const PathName& pathArg, bool readWrite)
{
  path = pathArg;
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

void* unxMemoryMappedFile::Resize(size_t newSize)
{
  DestroyMapping();
  if (ftruncate(filedes, newSize) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("ftruncate", "path", path.ToString());
  }
  CreateMapping(newSize);
  return ptr;
}

void unxMemoryMappedFile::OpenFile()
{
  int oflag(readWrite ? O_RDWR : O_RDONLY);
  filedes = open(path.GetData(), oflag);
  if (filedes < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("open", "path", path.ToString(), "readWrite", readWrite ? "true" : "false");
  }
}

void unxMemoryMappedFile::CreateMapping(size_t maximumFileSize)
{
  struct stat statbuf;

  if (fstat(filedes, &statbuf) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("fstat", "path", path.ToString());
  }

  if (statbuf.st_size == 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("File is empty."), "path", path.ToString());
  }

  if (maximumFileSize == 0)
  {
    maximumFileSize = statbuf.st_size;
  }

  size = maximumFileSize;

  ptr = mmap(nullptr, size, (readWrite ? (PROT_READ | PROT_WRITE) : PROT_READ), MAP_SHARED, filedes, 0);

  if (ptr == MAP_FAILED)
  {
    MIKTEX_FATAL_CRT_ERROR_2("mmap", "path", path.ToString(), "size", std::to_string(size), "readWrite", std::to_string(readWrite));
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
    MIKTEX_FATAL_CRT_ERROR_2("close", "path", path.ToString());
  }
}

void unxMemoryMappedFile::DestroyMapping()
{
  if (ptr == nullptr)
  {
    return;
  }
  void* ptr = this->ptr;
  this->ptr = nullptr;
  if (munmap(ptr, size) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("munmap", "path", path.ToString());
  }
}

void unxMemoryMappedFile::Flush()
{
  if (msync(ptr, size, MS_SYNC) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("msync", "path", path.ToString());
  }
}
