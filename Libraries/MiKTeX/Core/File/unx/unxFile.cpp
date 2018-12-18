/* unxFile.cpp: file operations

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

#include <thread>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#if defined(HAVE_SYS_TIME_H)
#  include <sys/time.h>
#endif

#include "internal.h"

#include "miktex/Core/Directory.h"
#include "miktex/Core/File.h"
#include "miktex/Core/FileStream.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

mode_t GetFileCreationMask()
{
#if 0
  // not atomic
  mode_t cmask = umask(0);
  umask(cmask);
  return cmask;
#else
  return 022;
#endif
}

bool File::Exists(const PathName& path, FileExistsOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  struct stat statbuf;
  int statret;
  if (options[FileExistsOption::SymbolicLink])
  {
    statret = lstat(path.GetData(), &statbuf);
  }
  else
  {
    statret = stat(path.GetData(), &statbuf);
  }    
  if (statret == 0)
  {
    if (S_ISDIR(statbuf.st_mode) != 0)
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
  int error = errno;
  if (error != ENOENT)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", path.ToString());
  }
  if (session != nullptr)
  {
    session->trace_access->WriteFormattedLine("core", T_("accessing file %s: NOK"), Q_(path));
  }
  return false;
}

FileAttributeSet File::GetAttributes(const PathName& path)
{
  mode_t attributes = static_cast<mode_t>(GetNativeAttributes(path));

  FileAttributeSet result;

  if (S_ISDIR(attributes) != 0)
  {
    result += FileAttribute::Directory;
  }

  if (((attributes & S_IWUSR) == 0)
    && ((attributes & S_IWGRP) == 0)
    && ((attributes & S_IWOTH) == 0))
  {
    result += FileAttribute::ReadOnly;
  }

  if (((attributes & S_IXUSR) != 0)
    || ((attributes & S_IXGRP) != 0)
    || ((attributes & S_IXOTH) != 0))
  {
    result += FileAttribute::Executable;
  }
  
  return result;
}

unsigned long File::GetNativeAttributes(const PathName& path)
{
  struct stat statbuf;

  if (stat(path.GetData(), &statbuf) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", path.ToString());
  }

  return static_cast<unsigned long>(statbuf.st_mode);
}

void File::SetAttributes(const PathName& path, FileAttributeSet attributes)
{
  mode_t newAttributes = S_IRUSR | S_IRGRP | S_IROTH;
  if (!attributes[FileAttribute::ReadOnly])
  {
    newAttributes |= S_IWUSR | S_IWGRP | S_IWOTH;
  }
  if (attributes[FileAttribute::Executable])
  {
    newAttributes |= S_IXUSR | S_IXGRP | S_IXOTH;
  }
  newAttributes &= ~GetFileCreationMask();
  if (newAttributes != static_cast<unsigned long>(GetNativeAttributes(path)))
  {
    SetNativeAttributes(path, static_cast<unsigned long>(newAttributes));
  }
}

void File::SetNativeAttributes(const PathName& path, unsigned long nativeAttributes)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("setting new attributes (%x) on %s"), static_cast<int>(nativeAttributes), Q_(path));
  }
  if (chmod(path.GetData(), static_cast<mode_t>(nativeAttributes)) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("chmod", "path", path.ToString());
  }
}

size_t File::GetSize(const PathName& path)
{
  struct stat statbuf;
  if (stat(path.GetData(), &statbuf) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", path.ToString());
  }
  return statbuf.st_size;
}

void File::SetTimes(int fd, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  UNUSED_ALWAYS(creationTime);
  time_t now = time(nullptr);
  if (lastAccessTime == static_cast<time_t>(-1))
  {
    lastAccessTime = now;
  }
  if (lastWriteTime == static_cast<time_t>(-1))
  {
    lastWriteTime = now;
  }
#if defined(HAVE_FUTIMES)
  timeval times[2] = {
    { lastAccessTime, 0 },
    { lastWriteTime, 0 }
  };
  if (futimes(fd, times) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("futimes");
  }
#else
  UNUSED_ALWAYS(fd);
  UNIMPLEMENTED();
#endif
}

void File::SetTimes(FILE* stream, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  int fd = fileno(stream);
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fileno");
  }    
  SetTimes(fd, creationTime, lastAccessTime, lastWriteTime);
}

void File::SetTimes(const PathName& path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime)
{
  UNUSED_ALWAYS(creationTime);
  utimbuf times;
  time_t now = time(nullptr);
  if (lastAccessTime == static_cast<time_t>(-1))
  {
    lastAccessTime = now;
  }
  if (lastWriteTime == static_cast<time_t>(-1))
  {
    lastWriteTime = now;
  }
  times.actime = lastAccessTime;
  times.modtime = lastWriteTime;
  if (utime(path.GetData(), &times) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("utime", "path", path.ToString());
  }
}

void File::GetTimes(const PathName& path, time_t& creationTime, time_t& lastAccessTime, time_t& lastWriteTime)
{
  struct stat stat_;
  if (stat(path.GetData(), &stat_) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", path.ToString());
  }
  creationTime = stat_.st_ctime;
  lastAccessTime = stat_.st_atime;
  lastWriteTime = stat_.st_mtime;
}

void File::Delete(const PathName& path)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("deleting %s"), Q_(path));
  }
  if (remove(path.GetData()) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("remove", "path", path.ToString());
  }
}

void File::Move(const PathName& source, const PathName& dest, FileMoveOptionSet options)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("renaming %s to %s"), Q_(source), Q_(dest));
  }
  struct stat sourceStat;
  if (stat(source.GetData(), &sourceStat) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", source.ToString());
  }
  PathName destDir(dest);
  destDir.MakeAbsolute();
  destDir.RemoveFileSpec();
  struct stat destStat;
  if (stat(destDir.GetData(), &destStat) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("stat", "path", destDir.ToString());
  }
  bool sameDevice = sourceStat.st_dev == destStat.st_dev;
  if (sameDevice)
  {
    if (options[FileMoveOption::ReplaceExisting] && File::Exists(dest))
    {
      File::Delete(dest);
    }
    if (rename(source.GetData(), dest.GetData()) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("rename", "source", source.ToString(), "dest", dest.ToString());
    }
  }
  else
  {
    Copy(source, dest);
    try
    {
      Delete(source);
    }
    catch (const MiKTeXException &)
    {
      try
      {
        if (Exists(source))
        {
          Delete(dest);
        }
      }
      catch (const MiKTeXException &)
      {
      }
      throw;
    }
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
      Fndb::Add({ { dest } });
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
  struct stat sourceStat;
  if (options[FileCopyOption::PreserveAttributes])
  {
    if (stat(source.GetData(), &sourceStat) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("stat", "path", source.ToString());
    }
  }
  FileStream sourceStream(File::Open(source, FileMode::Open, FileAccess::Read, false));
  try
  {
    FileStream destStream(File::Open(dest, FileMode::Create, FileAccess::Write, false));
    char buffer[4096];
    size_t n;
    while ((n = sourceStream.Read(buffer, 4096)) > 0)
    {
      destStream.Write(buffer, n);
    }
    sourceStream.Close();
    destStream.Close();
    if (options[FileCopyOption::PreserveAttributes])
    {
      SetNativeAttributes(dest, static_cast<unsigned long>(sourceStat.st_mode));
#if defined(HAVE_CHOWN)
      if (chown(dest.GetData(), sourceStat.st_uid, sourceStat.st_gid) != 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("chown", "path", dest.ToString());
      }
#endif
      SetTimes(dest.GetData(), sourceStat.st_ctime, sourceStat.st_atime, sourceStat.st_mtime);
    }
  }
  catch (const MiKTeXException &)
  {
    try
    {
      if (Exists(dest))
      {
        Delete(dest, { FileDeleteOption::TryHard });
      }
    }
    catch (const MiKTeXException &)
    {
    }
    throw;
  }
  if (options[FileCopyOption::UpdateFndb])
  {
    MIKTEX_EXPECT(session != nullptr);
    if (session->IsTEXMFFile(dest) && !Fndb::FileExists(dest))
    {
      Fndb::Add({ { dest } });
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
    session->trace_files->WriteFormattedLine("core", T_("creating %s link from %s to %s"), options[CreateLinkOption::Symbolic] ? "symbolic" : "hard",  Q_(newName), Q_(oldName));
  }
  if (options[CreateLinkOption::Symbolic])
  {
    if (symlink(oldName.GetData(), newName.GetData()) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("symlink", "oldName", oldName.ToString(), "newName", newName.ToString());
    }
  }
  else
  {
    if (link(oldName.GetData(), newName.GetData()) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("link", "oldName", oldName.ToString(), "newName", newName.ToString());
    }
  }
  if (options[CreateLinkOption::UpdateFndb])
  {
    MIKTEX_EXPECT(session != nullptr);
    if (session->IsTEXMFFile(newName) && !Fndb::FileExists(newName))
    {
      Fndb::Add({ { newName } });
    }
  }
}

bool File::IsSymbolicLink(const PathName& path)
{
  struct stat statbuf;
  if (lstat(path.GetData(), &statbuf) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("lstat", "path", path.ToString());
  }
  return S_ISLNK(statbuf.st_mode);
}

PathName File::ReadSymbolicLink(const PathName& path)
{
  PathName result;
  ssize_t len = readlink(path.GetData(), result.GetData(), result.GetCapacity());
  if (len < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("readlink", "path", path.ToString());
  }
  if (len == result.GetCapacity())
  {
    BUF_TOO_SMALL();
  }
  result[len] = 0;
  return result;
}

size_t File::SetMaxOpen(size_t newMax)
{
  // FIXME: unimplemented
  return FOPEN_MAX;
}

FILE* File::Open(const PathName& path, FileMode mode, FileAccess access, bool isTextFile, FileOpenOptionSet options)
{
  UNUSED_ALWAYS(isTextFile);

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  if (session != nullptr)
  {
    session->trace_files->WriteFormattedLine("core", T_("opening file %s (%d %d %d)"), Q_(path), (int)mode, (int)access, (int)isTextFile);
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
    flags |= O_APPEND;
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

  if (mode == FileMode::Create)
  {
    PathName dir(path);
    dir.MakeAbsolute();
    dir.RemoveFileSpec();
    if (!Directory::Exists(dir))
    {
      Directory::Create(dir);
    }
  }

  int fd;

  fd = open(path.GetData(), flags, (((flags & O_CREAT) == 0) ? 0 : (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)));

  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("open", "path", path.ToString());
  }
  
  try
  {
    if (options[FileOpenOption::DeleteOnClose])
    {
      File::Delete(path);
    }
    return FdOpen(fd, strFlags.c_str());
  }
  catch (const exception &)
  {
    close(fd);
    throw;
  }
}


bool File::TryLock(FILE *file, File::LockType lockType, chrono::milliseconds timeout)
{
  chrono::time_point<chrono::high_resolution_clock> tryUntil = chrono::high_resolution_clock::now() + timeout;
  bool locked;
  do
  {
    locked = flock(fileno(file), (lockType == LockType::Exclusive ? LOCK_EX : LOCK_SH) | LOCK_NB) == 0;
    if (!locked)
    {
      if (errno != EWOULDBLOCK)
      {
        MIKTEX_FATAL_CRT_ERROR("flock");
      }
      this_thread::sleep_for(10ms);
    }
  } while (!locked && chrono::high_resolution_clock::now() < tryUntil);
  return locked;
}

void File::Unlock(FILE *file)
{
  if (flock(fileno(file), LOCK_UN) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("flock");
  }
}
