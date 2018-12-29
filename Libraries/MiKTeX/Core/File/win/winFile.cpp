/* File.cpp: file operations

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

#include <thread>

#include <fcntl.h>

#include <io.h>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/win/winAutoResource>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

static unsigned long GetFileAttributes_harmlessErrors[] = {
  ERROR_FILE_NOT_FOUND, // 2
  ERROR_PATH_NOT_FOUND, // 3
  ERROR_NOT_READY, // 21
  ERROR_BAD_NETPATH, // 53
  ERROR_BAD_NET_NAME, // 67
  ERROR_INVALID_NAME, // 123
  ERROR_BAD_PATHNAME, // 161
};

bool File::Exists(const PathName& path, FileExistsOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (options[FileExistsOption::SymbolicLink])
  {
    UNIMPLEMENTED();
  }
  unsigned long attributes = GetFileAttributesW(path.ToWideCharString().c_str());
  if (attributes != INVALID_FILE_ATTRIBUTES)
  {
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
      if (session != nullptr)
      {
        session->trace_access->WriteFormattedLine("core", T_("%s is a directory"), Q_(path));
      }
      return false;
    }
    if (session != nullptr)
    {
      session->trace_access->WriteFormattedLine("core", T_("accessing file %s: OK"), Q_(path));
    }
    return true;
  }
  unsigned long error = ::GetLastError();
  // TODO: range-based for loop
  for (int idx = 0; idx < sizeof(GetFileAttributes_harmlessErrors) / sizeof(GetFileAttributes_harmlessErrors[0]); ++idx)
  {
    if (error == GetFileAttributes_harmlessErrors[idx])
    {
      error = ERROR_SUCCESS;
      break;
    }
  }
  if (error != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_3("GetFileAttributesW",
      T_("MiKTeX cannot retrieve attributes for the file '{path}'."),
      "path", path.ToDisplayString());
  }
  if (session != nullptr)
  {
    session->trace_access->WriteFormattedLine("core", T_("accessing file %s: NOK"), Q_(path));
  }
  return false;
}

FileAttributeSet File::GetAttributes(const PathName& path)
{
  unsigned long attributes = GetNativeAttributes(path);

  FileAttributeSet result;

  if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
  {
    result += FileAttribute::Directory;
  }

  if ((attributes & FILE_ATTRIBUTE_READONLY) != 0)
  {
    result += FileAttribute::ReadOnly;
  }

  if ((attributes & FILE_ATTRIBUTE_HIDDEN) != 0)
  {
    result += FileAttribute::Hidden;
  }

  return result;
}

unsigned long File::GetNativeAttributes(const PathName& path)
{
  unsigned long attributes = GetFileAttributesW(path.ToWideCharString().c_str());

  if (attributes == INVALID_FILE_ATTRIBUTES)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetFileAttributesW", "path", path.ToString());
  }

  return attributes;
}

void File::SetAttributes(const PathName& path, FileAttributeSet attributes)
{
  unsigned long attributesOld = GetNativeAttributes(path);

  unsigned long attributesNew = attributesOld;

  if (attributes[FileAttribute::ReadOnly])
  {
    attributesNew |= FILE_ATTRIBUTE_READONLY;
  }
  else
  {
    attributesNew &= ~FILE_ATTRIBUTE_READONLY;
  }

  if (attributes[FileAttribute::Hidden])
  {
    attributesNew |= FILE_ATTRIBUTE_HIDDEN;
  }
  else
  {
    attributesNew &= ~FILE_ATTRIBUTE_HIDDEN;
  }

  if (attributesNew == attributesOld)
  {
    return;
  }

  SetNativeAttributes(path, attributesNew);
}

void File::SetNativeAttributes(const PathName& path, unsigned long nativeAttributes)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("setting new attributes (%x) on %s"), static_cast<int>(nativeAttributes), Q_(path));
  }

  if (!SetFileAttributesW(path.ToWideCharString().c_str(), static_cast<DWORD>(nativeAttributes)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_3("SetFileAttributesW",
      T_("MiKTeX cannot set attributes for file or directory '{path}'."),
      "path", path.ToDisplayString(),
      "attributes", std::to_string(nativeAttributes));
  }
}

size_t File::GetSize(const PathName& path)
{
  HANDLE h = CreateFileW(path.ToWideCharString().c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (h == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
  }

  AutoHANDLE autoClose(h);

  // TODO: large file support
  unsigned long fileSize = GetFileSize(h, nullptr);

  if (fileSize == INVALID_FILE_SIZE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetFileSize", "path", path.ToString());
  }

  return fileSize;
}

// number of 100 nanosecond units from 1/1/1601 to 1/1/1970
const LONGLONG EPOCH_BIAS = 116444736000000000;


// 1601-01-01 00:00:00 as Unx time
const LONGLONG MIN_TIME_T = -11644473600;

// 30827-12-31 23:59:59 as Unx time
const LONGLONG MAX_TIME_T = 910670515199;

MIKTEXSTATICFUNC(FILETIME) UniversalCrtTimeToFileTime(time_t time)
{
  FILETIME fileTime;
  if (time == static_cast<time_t>(-1)
    || time < MIN_TIME_T
    || time > MAX_TIME_T)

  {
    fileTime.dwLowDateTime = 0;
    fileTime.dwHighDateTime = 0;
  }
  else
  {
    LONGLONG ll = static_cast<LONGLONG>(time) * 10000000 + EPOCH_BIAS;
    fileTime.dwLowDateTime = static_cast<DWORD>(ll);
    fileTime.dwHighDateTime = ll >> 32;
  }
  return fileTime;
}

MIKTEXSTATICFUNC(time_t) FileTimeToUniversalCrtTime(FILETIME fileTime)
{
  if (fileTime.dwLowDateTime == 0 && fileTime.dwHighDateTime == 0)
  {
    return static_cast<time_t>(-1);
  }
  ULARGE_INTEGER uli;
  uli.LowPart = fileTime.dwLowDateTime;
  uli.HighPart = fileTime.dwHighDateTime;
  return (uli.QuadPart / 10000000) - (EPOCH_BIAS / 10000000);
}

MIKTEXINTERNALFUNC(void) SetTimesInternal(HANDLE handle, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  FILETIME creationFileTime;
  FILETIME lastAccessFileTime;
  FILETIME lastWriteFileTime;
  if (creationTime != static_cast<time_t>(-1))
  {
    creationFileTime = UniversalCrtTimeToFileTime(creationTime);
  }
  if (lastAccessTime != static_cast<time_t>(-1))
  {
    lastAccessFileTime = UniversalCrtTimeToFileTime(lastAccessTime);
  }
  if (lastWriteTime != static_cast<time_t>(-1))
  {
    lastWriteFileTime = UniversalCrtTimeToFileTime(lastWriteTime);
  }
  if (!SetFileTime(handle,
    creationTime != static_cast<time_t>(-1) ? &creationFileTime : nullptr,
    lastAccessTime != static_cast<time_t>(-1) ? &lastAccessFileTime : nullptr,
    lastWriteTime != static_cast<time_t>(-1) ? &lastWriteFileTime : nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetFileTime");
  }
}

#define GET_OSFHANDLE(hf) \
  reinterpret_cast<HANDLE>(_get_osfhandle(static_cast<int>(hf)))

void File::SetTimes(int fd, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  SetTimesInternal(GET_OSFHANDLE(fd), creationTime, lastAccessTime, lastWriteTime);
}

void File::SetTimes(FILE* stream, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  SetTimes(_fileno(stream), creationTime, lastAccessTime, lastWriteTime);
}

void File::SetTimes(const PathName& path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  FileStream stream(File::Open(path, FileMode::Open, FileAccess::ReadWrite, false));
  SetTimes(stream.GetFile(), creationTime, lastAccessTime, lastWriteTime);
  stream.Close();
}

void File::GetTimes(const PathName& path, time_t& creationTime, time_t& lastAccessTime, time_t& lastWriteTime)
{
  WIN32_FIND_DATAW findData;
  HANDLE findHandle = FindFirstFileW(path.ToWideCharString().c_str(), &findData);
  if (findHandle == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("FindFirstFileW", "path", path.ToString());
  }
  if (!FindClose(findHandle))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("FindClose", "path", path.ToString());
  }
  creationTime = FileTimeToUniversalCrtTime(findData.ftCreationTime);
  lastAccessTime = FileTimeToUniversalCrtTime(findData.ftLastAccessTime);
  lastWriteTime = FileTimeToUniversalCrtTime(findData.ftLastWriteTime);
}

void File::Delete(const PathName& path)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("deleting %s"), Q_(path));
  }
  if (!DeleteFileW(path.ToWideCharString().c_str()))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_3("DeleteFileW",
      T_("MiKTeX could not remove the file '{path}'."),
      "path", path.ToDisplayString());
  }
}

void File::Move(const PathName& source, const PathName& dest, FileMoveOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("renaming %s to %s"), Q_(source), Q_(dest));
  }
  DWORD flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
  if (options[FileMoveOption::ReplaceExisting])
  {
    flags |= MOVEFILE_REPLACE_EXISTING;
  }
  if (!MoveFileExW(source.ToWideCharString().c_str(), dest.ToWideCharString().c_str(), flags))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_3("MoveFileExW",
      T_("MiKTeX could not rename the file '{existing}'."),
      "existing", source.ToDisplayString(),
      "path", dest.ToDisplayString());
  }
  if (options[FileMoveOption::UpdateFndb])
  {
    MIKTEX_EXPECT(session != nullptr);
    if (session->IsTEXMFFile(source) && Fndb::FileExists(source))
    {
      Fndb::Remove({ source });
    }
    if (session->IsTEXMFFile(dest) && !Fndb::FileExists(dest))
    {
      Fndb::Add({ {dest} });
    }
  }
}

void File::Copy(const PathName& source, const PathName& dest, FileCopyOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("copying %s to %s"), Q_(source), Q_(dest));
  }
  if (options[FileCopyOption::ReplaceExisting] && File::Exists(dest))
  {
    unsigned long destAttributes = GetNativeAttributes(dest);
    unsigned long destAttributes2 = destAttributes;
    destAttributes2 &= ~FILE_ATTRIBUTE_READONLY;
    destAttributes2 &= ~FILE_ATTRIBUTE_HIDDEN;
    if (destAttributes != destAttributes2)
    {
      SetNativeAttributes(dest, destAttributes2);
    }
  }
  if (!CopyFileW(source.ToWideCharString().c_str(), dest.ToWideCharString().c_str(), options[FileCopyOption::ReplaceExisting] ? FALSE : TRUE))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CopyFileW", "existing", source.ToString(), "path", dest.ToString());
  }
  if (options[FileCopyOption::UpdateFndb])
  {
    MIKTEX_EXPECT(session != nullptr);
    if (session->IsTEXMFFile(dest) && !Fndb::FileExists(dest))
    {
      Fndb::Add({ {dest} });
    }
  }
}

void File::CreateLink(const PathName& oldName, const PathName& newName, CreateLinkOptionSet options)
{
  if (options[CreateLinkOption::ReplaceExisting] && File::Exists(newName) )
  {
    FileDeleteOptionSet deleteOptions = { FileDeleteOption::TryHard };
    if (options[CreateLinkOption::UpdateFndb])
    {
      deleteOptions += FileDeleteOption::UpdateFndb;
    }
    File::Delete(newName, deleteOptions);
  }
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("creating %s link from %s to %s"), options[CreateLinkOption::Symbolic] ? "symbolic" : "hard", Q_(newName), Q_(oldName));
  }
  if (options[CreateLinkOption::Symbolic])
  {
    UNIMPLEMENTED();
  }
  else if (CreateHardLinkW(newName.ToWideCharString().c_str(), oldName.ToWideCharString().c_str(), nullptr) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateHardLinkW", "path", newName.ToString(), "existing", oldName.ToString());
  }
  if (options[CreateLinkOption::UpdateFndb])
  {
    MIKTEX_EXPECT(session != nullptr);
    if (session->IsTEXMFFile(newName) && !Fndb::FileExists(newName))
    {
      Fndb::Add({ {newName} });
    }
  }
}

bool File::IsSymbolicLink(const PathName& path)
{
  UNIMPLEMENTED();
}

PathName File::ReadSymbolicLink(const PathName& path)
{
  UNIMPLEMENTED();
}

size_t File::SetMaxOpen(size_t newMax)
{
  newMax = min(newMax, 2048);
  int oldMax = _getmaxstdio();
  if (oldMax >= newMax)
  {
    newMax = oldMax;
  }
  else
  {
    shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
    if (session != nullptr)
    {
      session->trace_files->WriteFormattedLine("core", T_("increasing maximum number of simultaneously open files (oldmax=%d, newmax=%d)"), (int)oldMax, (int)newMax);
    }
    if (_setmaxstdio(static_cast<int>(newMax)) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("_setmaxstdio", "newmax", std::to_string(newMax));
    }
  }
  return newMax;
}

FILE* File::Open(const PathName& path, FileMode mode, FileAccess access, bool isTextFile, FileOpenOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("opening file %s (%d 0x%x %d)"), Q_(path), static_cast<int>(mode), static_cast<int>(access), static_cast<int>(isTextFile));
  }

  int flags = 0;
  string strFlags;

  if (mode == FileMode::Create)
  {
    flags |= O_CREAT;
  }
  else if (mode == FileMode::CreateNew)
  {
    flags |= O_CREAT | O_EXCL;
  }
  else if (mode == FileMode::Append)
  {
    flags |= O_CREAT | O_APPEND;
  }

  if (access == FileAccess::ReadWrite)
  {
    flags |= O_RDWR;
    if (mode == FileMode::Append)
    {
      strFlags += "a+";
    }
    else
    {
      strFlags += "r+";
    }
  }
  else if (access == FileAccess::Read)
  {
    flags |= O_RDONLY;
    strFlags += "r";
  }
  else if (access == FileAccess::Write)
  {
    flags |= O_WRONLY;
    if (mode == FileMode::Append)
    {
      strFlags += "a";
    }
    else
    {
      flags |= O_TRUNC;
      strFlags += "w";
    }
  }

  if (options[FileOpenOption::DeleteOnClose])
  {
    flags |= O_TEMPORARY;
  }

#if defined(O_SEQUENTIAL)
  flags |= O_SEQUENTIAL;
#if 0
  strFlags += "S";
#endif
#endif

  if (isTextFile)
  {
    flags |= O_TEXT;
    strFlags += "t";
  }
  else
  {
    flags |= O_BINARY;
    strFlags += "b";
  }

  if (mode == FileMode::Create || mode == FileMode::CreateNew || mode == FileMode::Append)
  {
    PathName dir(path);
    dir.MakeAbsolute();
    dir.RemoveFileSpec();
    if (!Directory::Exists(dir))
    {
      Directory::Create(dir);
    }
  }

  int fd = _wopen(path.ToWideCharString().c_str(), flags, ((flags & O_CREAT) == 0) ? 0 : S_IREAD | S_IWRITE);
  if (fd < 0)
  {
    if (errno == EINVAL && ::GetLastError() == ERROR_USER_MAPPED_FILE)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString(), "modeString", strFlags);
    }
    else
    {
      MIKTEX_FATAL_CRT_ERROR_2("_wopen", "path", path.ToString(), "modeString", strFlags);
    }
  }

  return FdOpen(path, fd, strFlags.c_str());
}

bool File::TryLock(HANDLE hFile, File::LockType lockType, chrono::milliseconds timeout)
{
  chrono::time_point<chrono::high_resolution_clock> tryUntil = chrono::high_resolution_clock::now() + timeout;
  bool locked;
  do
  {
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    locked = LockFileEx(hFile, (lockType == LockType::Exclusive ? LOCKFILE_EXCLUSIVE_LOCK : 0) | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD, MAXDWORD, &overlapped) ? true : false;
    if (!locked)
    {
      if (GetLastError() != ERROR_LOCK_VIOLATION)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("LockFileEx");
      }
      this_thread::sleep_for(10ms);
    }
  } while (!locked && chrono::high_resolution_clock::now() < tryUntil);
  return locked;
}

bool File::TryLock(int fd, File::LockType lockType, chrono::milliseconds timeout)
{
  return TryLock(reinterpret_cast<HANDLE>(_get_osfhandle(fd)), lockType, timeout);
}

void File::Unlock(HANDLE hFile)
{
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(overlapped));
  if (!UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("UnlockFileEx");
  }
}

void File::Unlock(int fd)
{
  Unlock(reinterpret_cast<HANDLE>(_get_osfhandle(fd)));
}
