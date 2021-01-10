/* winMemoryMappedFile.cpp: memory mapped files

   Copyright (C) 1996-2021 Christian Schenk

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/BufferSizes>
#include <miktex/Trace/Trace>
#include <miktex/Util/PathNameUtil>

#include "internal.h"

#include "winMemoryMappedFile.h"
#include "Utils/inliners.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

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

void* winMemoryMappedFile::Open(const PathName& path_, bool readWrite)
{
  path = path_;
  this->readWrite = readWrite;

  // create a unique object name
  PathName path2 = path;
  path2.TransformForComparison();
  name = "";
  for (size_t i = 0; path2[i] != 0; ++i)
  {
    if (PathNameUtil::IsDirectoryDelimiter(path2[i]) || path2[i] == ':')
    {
      continue;
    }
    name +=path2[i];
  }

  // try to open an existing file mapping
  hMapping = OpenFileMappingW(readWrite ? FILE_MAP_WRITE : FILE_MAP_READ, FALSE, PathName(name).ToWideCharString().c_str());

  if (hMapping != nullptr)
  {
    traceStream->WriteLine("core", fmt::format(T_("using existing file mapping object {0}"), Q_(name)));

    // map existing file view into memory
    ptr = MapViewOfFile(hMapping, (readWrite ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, 0);
    if (ptr == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_5("MapViewOfFile",
        T_("MiKTeX cannot access the file '{path}'. It might be in use (blocked by another program)."),
        T_("Close other programs and try again."),
        "file-in-use",
        "path", path.ToDisplayString(),
        "readWrite", std::to_string(readWrite));
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
    DWORD lastError = GetLastError();
    if (lastError != ERROR_FILE_NOT_FOUND)
    {
      MIKTEX_FATAL_WINDOWS_RESULT_2("OpenFileMappingW", lastError, "path", name);
    }

    traceStream->WriteLine("core", fmt::format(T_("creating new file mapping object {0}"), Q_(name)));

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

  traceStream->WriteLine("core", fmt::format(T_("opening memory-mapped file {0} for {1}"), Q_(path), (readWrite ? T_("reading/writing") : T_("reading"))));

  hFile = CreateFileW(UW_(path.GetData()), desiredAccess, shareMode, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
  }

  if (!File::TryLock(hFile, readWrite ? File::LockType::Exclusive : File::LockType::Shared, 10ms))
  {
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    MIKTEX_FATAL_ERROR_2(T_("Could not acquire lock."), "path", path.ToString());
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
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileMappingW", "path", name);
  }

  // map file view into memory
  ptr = MapViewOfFile(hMapping, (readWrite ? FILE_MAP_WRITE : FILE_MAP_READ), 0, 0, maximumFileSize);
  if (ptr == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("MapViewOfFile", "path", name);
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
  traceStream->WriteLine("core", fmt::format(T_("closing memory-mapped file {0}"), Q_(path)));
  try
  {
    File::Unlock(hFile);
  }
  catch (const MiKTeXException&)
  {
    // TODO: logging
  }
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
