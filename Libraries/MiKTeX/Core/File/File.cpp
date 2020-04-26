/* File.cpp: file operations

   Copyright (C) 1996-2020 Christian Schenk

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

#include <random>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

bool File::Exists(const PathName& path)
{
  return File::Exists(path, {});
}

void File::Delete(const PathName& path, FileDeleteOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  if (options[FileDeleteOption::UpdateFndb])
  {
    if (session == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    if (session->IsTEXMFFile(path) && Fndb::FileExists(path))
    {
      Fndb::Remove({ path });
    }
  }

#if defined(MIKTEX_WINDOWS)
  FileAttributeSet attributes = File::GetAttributes(path);
#endif

  bool done;

  try
  {
#if defined(MIKTEX_WINDOWS)
    if (attributes[FileAttribute::ReadOnly])
    {
      attributes -= FileAttribute::ReadOnly;
      File::SetAttributes(path, attributes);
    }
#endif
    File::Delete(path);
    done = true;
  }

  catch (const UnauthorizedAccessException &)
  {
#if defined(MIKTEX_WINDOWS)
    if (!options[FileDeleteOption::TryHard])
    {
      throw;
    }
    if (session != nullptr)
    {
      session->trace_files->WriteLine("core", fmt::format(T_("file {0} is in use"), Q_(path)));
    }
    done = false;
#else
    throw;
#endif
  }

#if defined(MIKTEX_WINDOWS)
  if (!done)
  {
    // move the file out of the way
    PathName old = path;
    old.AppendExtension(MIKTEX_TO_BE_DELETED_FILE_SUFFIX);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 99999);
    size_t maxrounds = 10;
    while (--maxrounds > 0 && File::Exists(old))
    {
      old = path;
      old.AppendExtension(string(".") + std::to_string(dist(gen)) + MIKTEX_TO_BE_DELETED_FILE_SUFFIX);
    }
    File::Move(path, old, { FileMoveOption::ReplaceExisting });
    session->ScheduleFileRemoval(old);
  }
#endif
}

vector<unsigned char> File::ReadAllBytes(const PathName& path)
{
  size_t size = GetSize(path);
  vector<unsigned char> arr;
  arr.resize(size);
  FileStream stream(Open(path, FileMode::Open, FileAccess::Read, false));
  stream.Read(&arr[0], size);
  return arr;
}

void File::WriteBytes(const PathName& path, const vector<unsigned char>& data)
{
  ofstream stream = CreateOutputStream(path, ios_base::binary);
  stream.write((const char*)data.data(), data.size());
  stream.close();
}

FILE* File::Open(const PathName& path, FileMode mode, FileAccess access)
{
  return Open(path, mode, access, true);
}

FILE* File::Open(const PathName& path, FileMode mode, FileAccess access, bool isTextFile)
{
  return File::Open(path, mode, access, isTextFile, {});
}

std::ifstream File::CreateInputStream(const PathName& path, std::ios_base::openmode mode, std::ios_base::iostate exceptions)
{
  ifstream stream(path.ToNativeString(), mode);
  if (!stream.is_open())
  {
    MIKTEX_FATAL_CRT_ERROR_2("ifstream::open", "path", path.ToString());
  }
  stream.exceptions(exceptions);
  return stream;
}

std::ofstream File::CreateOutputStream(const PathName& path, std::ios_base::openmode mode, std::ios_base::iostate exceptions)
{
  PathName dir(path);
  dir.MakeAbsolute();
  dir.RemoveFileSpec();
  if (!Directory::Exists(dir))
  {
    Directory::Create(dir);
  }
  ofstream stream(path.ToNativeString(), mode);
  if (!stream.is_open())
  {
    MIKTEX_FATAL_CRT_ERROR_2("ofstream::open", "path", path.ToString());
  }
  stream.exceptions(exceptions);
  return stream;
}

bool File::Equals(const PathName& path1, const PathName& path2)
{
  size_t size = GetSize(path1);
  if (GetSize(path2) != size)
  {
    return false;
  }
  if (size == 0)
  {
    return true;
  }
  unique_ptr<MemoryMappedFile> file1(MemoryMappedFile::Create());
  const void* ptr1 = file1->Open(path1, false);
  unique_ptr<MemoryMappedFile> file2(MemoryMappedFile::Create());
  const void* ptr2 = file2->Open(path2, false);
  bool ret = memcmp(ptr1, ptr2, size) == 0;
  file1->Close();
  file2->Close();
  return ret;
}
