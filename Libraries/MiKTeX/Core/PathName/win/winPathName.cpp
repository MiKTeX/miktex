/* winPathName.cpp: path name utilities

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

#include "internal.h"

#include "miktex/Core/PathName.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

PathName& PathName::SetToCurrentDirectory()
{
  wchar_t buf[_MAX_PATH];
  if (_wgetcwd(buf, _MAX_PATH) == 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_wgetcwd");
  }
  *this = buf;
  return *this;
}

PathName& PathName::SetToTempDirectory()
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

PathName& PathName::SetToTempFile(const PathName& directory)
{
  wchar_t szTemp[MAX_PATH];
  UINT n = GetTempFileNameW(directory.ToWideCharString().c_str(), L"mik", 0, szTemp);
  if (n == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetTempFileNameW", "directory", directory.ToString());
  }
  *this = szTemp;
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_tempfile->WriteFormattedLine("core", T_("created temporary file %s"), Q_(GetData()));
  }
  return *this;
}

PathName& PathName::SetToTempFile()
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  PathName tmpDir;
  if (session != nullptr)
  {
    tmpDir = session->GetTempDirectory();
  }
  else
  {
    tmpDir.SetToTempDirectory();
  }
  return SetToTempFile(tmpDir);
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

PathName& PathName::AppendAltDirectoryDelimiter()
{
  size_t l = GetLength();
  if (l == 0 || !IsDirectoryDelimiter(CharBuffer::operator[](l - 1)))
  {
    CharBuffer::Append(AltDirectoryDelimiter);
  }
  return *this;
}
