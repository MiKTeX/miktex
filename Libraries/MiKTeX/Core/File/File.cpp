/* File.cpp: file operations

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

#include "miktex/Core/File.h"
#include "miktex/Core/Paths.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

void File::Delete(const PathName & path, FileDeleteOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();

  if (options[FileDeleteOption::UpdateFndb] && session->IsTEXMFFile(path) && Fndb::FileExists(path))
  {
    Fndb::Remove(path);
  }

  FileAttributeSet attributes = File::GetAttributes(path);

  bool done;

  try
  {
    if (attributes[FileAttribute::ReadOnly])
    {
      attributes -= FileAttribute::ReadOnly;
      File::SetAttributes(path, attributes);
    }
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
    session->trace_files->WriteFormattedLine("core", T_("file %s is in use"), Q_(path));
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
    size_t maxrounds = 10;
    while (--maxrounds > 0 && File::Exists(old))
    {
      unsigned unique = static_cast<unsigned>((static_cast<double>(rand()) / RAND_MAX) * 0xffff);
      old = path;
      old.AppendExtension(string(".") + std::to_string(unique) + MIKTEX_TO_BE_DELETED_FILE_SUFFIX);
    }
    File::Move(path, old, { FileMoveOption::ReplaceExisting });
    if (session->RunningAsAdministrator())
    {
      session->ScheduleFileRemoval(old.Get());
    }
    else
    {
      // TODO
    }
  }
#endif
}

vector<unsigned char> File::ReadAllBytes(const PathName & path)
{
  size_t size = GetSize(path);
  vector<unsigned char> arr;
  arr.resize(size);
  FileStream stream(Open(path, FileMode::Open, FileAccess::Read, false));
  stream.Read(&arr[0], size);
  return arr;
}

FILE * File::Open(const PathName & path, FileMode mode, FileAccess access)
{
  return Open(path, mode, access, true, FileShare::Read);
}

FILE * File::Open(const PathName & path, FileMode mode, FileAccess access, bool isTextFile)
{
  return Open(path, mode, access, isTextFile, FileShare::Read);
}

bool File::Equals(const PathName & path1, const PathName & path2)
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
  unique_ptr<MemoryMappedFile> pFile1(MemoryMappedFile::Create());
  const void * ptr1 = pFile1->Open(path1, false);
  unique_ptr<MemoryMappedFile> pFile2(MemoryMappedFile::Create());
  const void * ptr2 = pFile2->Open(path2, false);
  bool ret = memcmp(ptr1, ptr2, size) == 0;
  pFile1->Close();
  pFile2->Close();
  return ret;
}
