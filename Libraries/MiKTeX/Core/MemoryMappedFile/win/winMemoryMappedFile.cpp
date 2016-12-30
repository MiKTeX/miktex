/* winMemoryMappedFile.cpp: memory mapped files

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

#include "winMemoryMappedFile.h"
#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace std;

MemoryMappedFile* MemoryMappedFile::Create()
{
  return new winMemoryMappedFile;
}

winMemoryMappedFile::winMemoryMappedFile() :
  traceStream(TraceStream::Open(MIKTEX_TRACE_MMAP))
{
}

winMemoryMappedFile::~winMemoryMappedFile()
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

void* winMemoryMappedFile::Open(const PathName& pathArg, bool readWrite)
{
  path = pathArg;
  this->readWrite = readWrite;

  // create a unique object name
  name = "";
  name.reserve(BufferSizes::MaxPath);
  for (size_t i = 0; path[i] != 0; ++i)
  {
    if (IsDirectoryDelimiter(path[i]) || path[i] == ':')
    {
      continue;
    }
    name += ToLower(path[i]);
  }

  // try to open an existing file mapping
  hMapping = OpenFileMappingW(readWrite ? FILE_MAP_WRITE : FILE_MAP_READ, FALSE, PathName(name).ToWideCharString().c_str());

  if (hMapping != nullptr)
  {
    traceStream->WriteFormattedLine("core", T_("using existing file mapping object %s"), Q_(name));

    // map existing file view into memory
    ptr = MapViewOfFile(hMapping, (readWrite ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, 0);
    if (ptr == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("MapViewOfFile", "path", path.ToString());
    }

    // get the size
#if defined(_MSC_VER) || defined(__MINGW32__)
    struct _stat statbuf;
    if (_wstat(UW_(path.GetData()), &statbuf) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("_wstat", "path", path.ToString());
    }
    size = statbuf.st_size;
#else
#  error Unimplemented: winMemoryMappedFile::Open()
#endif
  }
  else
  {
    traceStream->WriteFormattedLine("core", T_("creating new file mapping object %s"), Q_(name));

    // create a new file mapping
    OpenFile();
    CreateMapping(0);
  }

  return ptr;
}

void winMemoryMappedFile::Close()
{
  DestroyMapping();
  CloseFile();
}

void* winMemoryMappedFile::Resize(size_t dwNewSize)
{
  DestroyMapping();
  CreateMapping(dwNewSize);
  return ptr;
}

void winMemoryMappedFile::OpenFile()
{
  unsigned long desiredAccess = GENERIC_READ;
  unsigned long shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

  if (readWrite)
  {
    desiredAccess |= GENERIC_WRITE;
  }

  traceStream->WriteFormattedLine("core", T_("opening memory-mapped file %s for %s"), Q_(path), (readWrite ? T_("reading/writing") : T_("reading")));

  hFile = CreateFileW(UW_(path.GetData()), desiredAccess, shareMode, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
  }
}

void winMemoryMappedFile::CreateMapping(size_t maximumFileSize)
{
  if (maximumFileSize == 0)
  {
    unsigned long fileSizeHigh;
    maximumFileSize = GetFileSize(hFile, &fileSizeHigh);
    if (maximumFileSize == INVALID_FILE_SIZE)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetFileSize", "path", path.ToString());
    }
    if (fileSizeHigh != 0)
    {
      MIKTEX_UNEXPECTED();
    }
    if (maximumFileSize == 0)
    {
      MIKTEX_UNEXPECTED();
    }
  }

  size = maximumFileSize;

  // create file-mapping object
  hMapping = ::CreateFileMappingW(hFile, nullptr, (readWrite ? PAGE_READWRITE : PAGE_READONLY), 0, static_cast<DWORD>(maximumFileSize), UW_(name));
  if (hMapping == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileMappingW", "name", name);
  }

  // map file view into memory
  ptr = MapViewOfFile(hMapping, (readWrite ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, maximumFileSize);
  if (ptr == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("MapViewOfFile", "name", name);
  }
}

void winMemoryMappedFile::CloseFile()
{
  if (this->hFile == INVALID_HANDLE_VALUE)
  {
    return;
  }
  HANDLE hFile = this->hFile;
  this->hFile = INVALID_HANDLE_VALUE;
  traceStream->WriteFormattedLine("core", T_("closing memory-mapped file %s"), Q_(path));
  if (!CloseHandle(hFile))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
  }
}

void winMemoryMappedFile::DestroyMapping()
{
  if (ptr != nullptr)
  {
    void* ptr = this->ptr;
    this->ptr = nullptr;
    if (!UnmapViewOfFile(ptr))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("UnmapViewOfFile");
    }
  }
  if (hMapping != nullptr)
  {
    HANDLE h = hMapping;
    hMapping = nullptr;
    if (!CloseHandle(h))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
    }
  }
}

void winMemoryMappedFile::Flush()
{
  if (!FlushViewOfFile(GetPtr(), 0))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("FlushViewOfFile");
  }
}
