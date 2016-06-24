/* winPathName.cpp: path name utilities

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

#include "miktex/Core/PathName.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;

void PathName::Combine(char * lpszPath, size_t sizePath, const char * lpszDrive, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
  if (_makepath_s(lpszPath, sizePath, lpszDrive, lpszAbsPath, lpszRelPath, lpszExtension) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_makepath_s");
  }
#else  // not Microsoft C++
#  error Unimplemented: PathName::Combine()
#endif  // not Microsoft C++
}

void PathName::Split(const char * lpszPath, char * lpszDrive, size_t sizeDrive, char * lpszDir, size_t sizeDir, char * lpszName, size_t sizeName, char * lpszExtension, size_t sizeExtension)
{
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszDrive, sizeDrive);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszDir, sizeDir);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszName, sizeName);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszExtension, sizeExtension);
#if defined(_MSC_VER) || defined(__MINGW32__)
  if (_splitpath_s(lpszPath, lpszDrive, sizeDrive, lpszDir, sizeDir, lpszName, sizeName, lpszExtension, sizeExtension) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_tsplitpath_s");
  }
#else  // not Microsoft C++
#  error Unimplemented: PathName::Split()
#endif  // not Microsoft C++
}

PathName & PathName::SetToCurrentDirectory()
{
  wchar_t buf[_MAX_PATH];
  if (_wgetcwd(buf, _MAX_PATH) == 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_wgetcwd");
  }
  *this = buf;
  return *this;
}

PathName & PathName::SetToTempDirectory()
{
  wchar_t szTemp[BufferSizes::MaxPath];
  unsigned long n =GetTempPathW(static_cast<DWORD>(BufferSizes::MaxPath), szTemp);
  if (n == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetTempPathW");
  }
  if (n >= GetCapacity())
  {
    MIKTEX_UNEXPECTED();
  }
  *this = szTemp;
  return *this;
}

PathName & PathName::SetToTempFile(const PathName & directory)
{
  wchar_t szTemp[MAX_PATH];
  UINT n = GetTempFileNameW(directory.ToWideCharString().c_str(), L"mik", 0, szTemp);
  if (n == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetTempFileNameW", "directory", directory.ToString());
  }
  *this = szTemp;
  SessionImpl::GetSession()->trace_tempfile->WriteFormattedLine("core", T_("created temporary file %s"), Q_(Get()));
  return *this;
}

PathName & PathName::SetToTempFile()
{
  return SetToTempFile(SessionImpl::GetSession()->GetTempDirectory());
}

PathName PathName::GetMountPoint() const
{
  wchar_t szDir[BufferSizes::MaxPath];
  if (!GetVolumePathNameW(this->ToWideCharString().c_str(), szDir, BufferSizes::MaxPath))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetVolumePathNameW", "path", this->ToString());
  }
  return szDir;
}

PathName & PathName::AppendAltDirectoryDelimiter()
{
  size_t l = GetLength();
  if (l == 0 || !IsDirectoryDelimiter(CharBuffer::operator[](l - 1)))
  {
    CharBuffer::Append(AltDirectoryDelimiter);
  }
  return *this;
}

