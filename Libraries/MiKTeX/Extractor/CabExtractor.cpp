/* CabExtractor.cpp:

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "config.h"

#if !defined(MIKTEX_WINDOWS)
#include <fcntl.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>

#include "internal.h"

#include "CabExtractor.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Trace;

struct mspack_file* CabExtractor::Open(struct mspack_system* self, const char* fileName, int mode)
{
  UNUSED_ALWAYS(self);
  try
  {
    MyFile* myFile = new MyFile;
    myFile->fileName = fileName;
    FileMode fileMode(FileMode::Open);
    FileAccess fileAccess(FileAccess::Read);
    switch (mode)
    {
    case MSPACK_SYS_OPEN_READ:
      fileMode = FileMode::Open;
      fileAccess = FileAccess::Read;
      break;
    case MSPACK_SYS_OPEN_WRITE:
      fileMode = FileMode::Create;
      fileAccess = FileAccess::Write;
      break;
    case MSPACK_SYS_OPEN_UPDATE:
      fileMode = FileMode::Open;
      fileAccess = FileAccess::ReadWrite;
      break;
    case MSPACK_SYS_OPEN_APPEND:
      fileMode = FileMode::Append;
      fileAccess = FileAccess::Write;
      break;
    default:
      delete myFile;
      MIKTEX_UNEXPECTED();
    }
    try
    {
      myFile->stdioFile = File::Open(fileName, fileMode, fileAccess, false);
    }
    catch (const exception&)
    {
      delete myFile;
      throw;
    }
    return reinterpret_cast<mspack_file*>(myFile);
  }
  catch (const exception&)
  {
    return 0;
  }
}

void CabExtractor::Close(struct mspack_file* mspackFile)
{
  MyFile* myFile = reinterpret_cast<MyFile*>(mspackFile);
  try
  {
    fclose(myFile->stdioFile);
    delete myFile;
  }
  catch (const exception&)
  {
  }
}

int CabExtractor::Read(struct mspack_file* mspackFile, void* data, int numBytes)
{
  MyFile* myFile = reinterpret_cast<MyFile*>(mspackFile);
  try
  {
    if (feof(myFile->stdioFile))
    {
      return 0;
    }
    size_t n = fread(data, 1, numBytes, myFile->stdioFile);
    if (ferror(myFile->stdioFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fread", "fileName", myFile->fileName);
    }
    return static_cast<int>(n);
  }
  catch (const exception&)
  {
    return -1;
  }
}

int CabExtractor::Write(struct mspack_file* mspackFile, void* data, int numBytes)
{
  MyFile* myFile = reinterpret_cast<MyFile*>(mspackFile);
  try
  {
    size_t n = fwrite(data, 1, numBytes, myFile->stdioFile);
    if (ferror(myFile->stdioFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fwrite", "fileName", myFile->fileName);
    }
    return static_cast<int>(n);
  }
  catch (const exception&)
  {
    return -1;
  }
}

int CabExtractor::Seek(struct mspack_file* mspackFile, off_t offset, int mode)
{
  MyFile* myFile = reinterpret_cast<MyFile*>(mspackFile);
  try
  {
    int origin;
    switch (mode)
    {
    case MSPACK_SYS_SEEK_START:
      origin = SEEK_SET;
      break;
    case MSPACK_SYS_SEEK_CUR:
      origin = SEEK_CUR;
      break;
    case MSPACK_SYS_SEEK_END:
      origin = SEEK_END;
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
    if (fseek(myFile->stdioFile, offset, origin) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fseek", "fileName", myFile->fileName);
    }
    return 0;
  }
  catch (const exception&)
  {
    return -1;
  }
}

off_t CabExtractor::Tell(struct mspack_file* mspackFile)
{
  MyFile* myFile = reinterpret_cast<MyFile*>(mspackFile);
  try
  {
    long position = ftell(myFile->stdioFile);
    if (position < 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("ftell", "fileName", myFile->fileName);
    }
    return position;
  }
  catch (const exception&)
  {
    return -1;
  }
}

void CabExtractor::Message(struct mspack_file* mspackFile, const char* format, ...)
{
  UNUSED_ALWAYS(mspackFile);
  UNUSED_ALWAYS(format);
}

void* CabExtractor::Alloc(struct mspack_system* self, size_t numBytes)
{
  UNUSED_ALWAYS(self);
  try
  {
    void* ptr = malloc(numBytes);
    if (ptr == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    return ptr;
  }
  catch (const exception&)
  {
    return nullptr;
  }
}

void CabExtractor::Free(void* ptr)
{
  free(ptr);
}

void CabExtractor::Copy(void* source, void* dest, size_t numBytes)
{
  memcpy(dest, source, numBytes);
}


CabExtractor::CabExtractor() :
  traceStream(TraceStream::Open(MIKTEX_TRACE_EXTRACTOR)),
  traceStopWatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH))
{
  mspackSystem.open = Open;
  mspackSystem.close = Close;
  mspackSystem.read = Read;
  mspackSystem.write = Write;
  mspackSystem.seek = Seek;
  mspackSystem.tell = Tell;
  mspackSystem.message = Message;
  mspackSystem.alloc = Alloc;
  mspackSystem.free = Free;
  mspackSystem.copy = Copy;
  mspackSystem.null_ptr = nullptr;
  decompressor = mspack_create_cab_decompressor(&mspackSystem);
  if (decompressor == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
}

CabExtractor::~CabExtractor()
{
  try
  {
    if (decompressor != nullptr)
    {
      mspack_destroy_cab_decompressor(decompressor);
      decompressor = nullptr;
    }
    if (traceStream.get() != nullptr)
    {
      traceStream->Close();
      traceStream.reset();
    }
    if (traceStopWatch.get() != nullptr)
    {
      traceStopWatch->Close();
      traceStopWatch.reset();
    }
  }
  catch (const exception&)
  {
  }
}

static void SetAttributes(const PathName& path, int cabattr)
{
  unsigned long nativeAttributes;
#if defined(MIKTEX_WINDOWS)
  nativeAttributes = 0;
  if ((cabattr & MSCAB_ATTRIB_RDONLY) != 0)
  {
    nativeAttributes |= FILE_ATTRIBUTE_READONLY;
  }
  if ((cabattr & MSCAB_ATTRIB_HIDDEN) != 0)
  {
    nativeAttributes |= FILE_ATTRIBUTE_HIDDEN;
  }
  if ((cabattr & MSCAB_ATTRIB_SYSTEM) != 0)
  {
    nativeAttributes |= FILE_ATTRIBUTE_SYSTEM;
  }
  if ((cabattr & MSCAB_ATTRIB_ARCH) != 0)
  {
    nativeAttributes |= FILE_ATTRIBUTE_ARCHIVE;
  }
  if (nativeAttributes == 0)
  {
    return;
  }
#else
  const unsigned long NORMAL =
    (0
      | S_IRUSR | S_IWUSR
      | S_IRGRP
      | S_IROTH);
  nativeAttributes = NORMAL;
  if ((cabattr & MSCAB_ATTRIB_RDONLY) != 0)
  {
    nativeAttributes &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
  }
  if ((cabattr & MSCAB_ATTRIB_EXEC) != 0)
  {
    nativeAttributes |= S_IXUSR;
  }
  if (nativeAttributes == NORMAL)
  {
    return;
  }
#endif
  File::SetNativeAttributes(path, nativeAttributes);
}

void CabExtractor::Extract(const PathName& cabinetPath, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(traceStopWatch.get(), TRACE_FACILITY, cabinetPath.GetFileName().ToString());
  traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracting {0} to {1} ({2})"), Q_(cabinetPath), Q_(destDir), (makeDirectories ? T_("make directories") : T_("don't make directories"))));

  mscabd_cabinet* cabinet = nullptr;

  try
  {
    cabinet = decompressor->open(decompressor, const_cast<char *>(cabinetPath.GetData()));

    if (cabinet == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The cabinet file could not be opened."), "path", cabinetPath.ToString());
    }

    size_t prefixLen = prefix.length();

    unsigned fileCount = 0;

    for (mscabd_file* cabFile = cabinet->files; cabFile != nullptr; cabFile = cabFile->next)
    {
      PathName dest(cabFile->filename);

#if defined(MIKTEX_UNIX)
      dest.ConvertToUnix();
#endif

      // skip directory prefix
      if (PathName::Compare(prefix, dest, prefixLen) == 0)
      {
        PathName tmp(dest);
        dest = tmp.GetData() + prefixLen;
      }

      // make the destination path name
      PathName path(destDir);
      if (!makeDirectories)
      {
        dest.RemoveDirectorySpec();
      }
      path /= dest;

      // notify the client
      if (callback != nullptr)
      {
        callback->OnBeginFileExtraction(path.ToString(), cabFile->length);
      }

      // create the destination directory
      Directory::Create(PathName(path).RemoveFileSpec());

      // remove the existing file
      if (File::Exists(path))
      {
        File::Delete(path, { FileDeleteOption::TryHard });
      }

      // extract the file
      int r = decompressor->extract(decompressor, cabFile, path.GetData());
      if (r != MSPACK_ERR_OK)
      {
         MIKTEX_FATAL_ERROR_2(T_("The member could not bex extracted from the cabinet file."), "cabinetPath", cabinetPath.ToString(), "member", cabFile->filename, "ret", std::to_string(r));
      }

      fileCount += 1;

      // set time when the file was created
      struct tm tm;
      tm.tm_sec = cabFile->time_s;
      tm.tm_min = cabFile->time_m;
      tm.tm_hour = cabFile->time_h;
      tm.tm_mday = cabFile->date_d;
      tm.tm_mon = cabFile->date_m - 1;
      tm.tm_year = cabFile->date_y - 1900;
      tm.tm_isdst = 0;
      time_t time = mktime(&tm);
      if (time == static_cast<time_t>(-1))
      {
        MIKTEX_FATAL_CRT_ERROR("mktime");
      }
      File::SetTimes(path, time, time, time);

      // set file attributes
      SetAttributes(path, cabFile->attribs);

      // notify the client
      if (callback != nullptr)
      {
        callback->OnEndFileExtraction("", cabFile->length);
      }
    }

    traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracted {0} file(s)"), fileCount));

    decompressor->close(decompressor, cabinet);
    cabinet = nullptr;

  }
  catch (const exception&)
  {
    if (cabinet != nullptr)
    {
      decompressor->close(decompressor, cabinet);
    }
    throw;
  }
}

void CabExtractor::Extract(Stream* stream, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
  UNUSED_ALWAYS(stream);
  UNUSED_ALWAYS(destDir);
  UNUSED_ALWAYS(makeDirectories);
  UNUSED_ALWAYS(callback);
  UNUSED_ALWAYS(prefix);
  UNIMPLEMENTED();
}
