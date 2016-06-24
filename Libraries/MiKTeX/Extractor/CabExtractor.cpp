/* CabExtractor.cpp:

   Copyright (C) 2001-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "CabExtractor.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Trace;
using namespace std;

struct mspack_file * CabExtractor::Open(struct mspack_system * self, const char * lpszFileName, int mode)
{
  UNUSED_ALWAYS(self);
  try
  {
    MyFile * pMyFile = new MyFile;
    pMyFile->fileName = lpszFileName;
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
      delete pMyFile;
      MIKTEX_UNEXPECTED();
    }
    try
    {
      pMyFile->pFile = File::Open(lpszFileName, fileMode, fileAccess, false);
    }
    catch (const exception &)
    {
      delete pMyFile;
      throw;
    }
    return reinterpret_cast<mspack_file*>(pMyFile);
  }
  catch (const exception &)
  {
    return 0;
  }
}

void CabExtractor::Close(struct mspack_file * pFile)
{
  MyFile * pMyFile = reinterpret_cast<MyFile*>(pFile);
  try
  {
    fclose(pMyFile->pFile);
    delete pMyFile;
  }
  catch (const exception &)
  {
  }
}

int CabExtractor::Read(struct mspack_file * pFile, void * pBuffer, int numBytes)
{
  MyFile * pMyFile = reinterpret_cast<MyFile*>(pFile);
  try
  {
    if (feof(pMyFile->pFile))
    {
      return 0;
    }
    size_t n = fread(pBuffer, 1, numBytes, pMyFile->pFile);
    if (ferror(pMyFile->pFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fread", "fileName", pMyFile->fileName);
    }
    return static_cast<int>(n);
  }
  catch (const exception &)
  {
    return -1;
  }
}

int CabExtractor::Write(struct mspack_file * pFile, void * pBuffer, int numBytes)
{
  MyFile * pMyFile = reinterpret_cast<MyFile*>(pFile);
  try
  {
    size_t n = fwrite(pBuffer, 1, numBytes, pMyFile->pFile);
    if (ferror(pMyFile->pFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fwrite", "fileName", pMyFile->fileName);
    }
    return static_cast<int>(n);
  }
  catch (const exception &)
  {
    return -1;
  }
}

int CabExtractor::Seek(struct mspack_file * pFile, off_t offset, int mode)
{
  MyFile * pMyFile = reinterpret_cast<MyFile*>(pFile);
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
    if (fseek(pMyFile->pFile, offset, origin) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("fseek", "fileName", pMyFile->fileName);
    }
    return 0;
  }
  catch (const exception &)
  {
    return -1;
  }
}

off_t CabExtractor::Tell(struct mspack_file * pFile)
{
  MyFile * pMyFile = reinterpret_cast<MyFile*>(pFile);
  try
  {
    long position = ftell(pMyFile->pFile);
    if (position < 0)
    {
      MIKTEX_FATAL_CRT_ERROR_2("ftell", "fileName", pMyFile->fileName);
    }
    return position;
  }
  catch (const exception &)
  {
    return -1;
  }
}

void CabExtractor::Message(struct mspack_file * pFile, const char * lpszFormat, ...)
{
  UNUSED_ALWAYS(pFile);
  UNUSED_ALWAYS(lpszFormat);
}

void * CabExtractor::Alloc(struct mspack_system * self, size_t numBytes)
{
  UNUSED_ALWAYS(self);
  try
  {
    void * ptr = malloc(numBytes);
    if (ptr == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    return ptr;
  }
  catch (const exception &)
  {
    return nullptr;
  }
}

void CabExtractor::Free(void * ptr)
{
  free(ptr);
}

void CabExtractor::Copy(void * pSource, void * pDest, size_t numBytes)
{
  memcpy(pDest, pSource, numBytes);
}


CabExtractor::CabExtractor() :
  traceStream(TraceStream::Open(MIKTEX_TRACE_EXTRACTOR))
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
  mspackSystem.null_ptr = 0;
  pDecompressor = mspack_create_cab_decompressor(&mspackSystem);
  if (pDecompressor == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
}

CabExtractor::~CabExtractor()
{
  try
  {
    if (pDecompressor != nullptr)
    {
      mspack_destroy_cab_decompressor(pDecompressor);
      pDecompressor = nullptr;
    }
    if (traceStream.get() != nullptr)
    {
      traceStream->Close();
      traceStream.reset();
    }
  }
  catch (const exception &)
  {
  }
}

static void SetAttributes(const PathName & path, int cabattr)
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

void CabExtractor::Extract(const PathName & cabinetPath, const PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const string & prefix)
{
  traceStream->WriteFormattedLine("libextractor", T_("extracting %s to %s (%s)"), Q_(cabinetPath), Q_(destDir), (makeDirectories ? T_("make directories") : T_("don't make directories")));

  mscabd_cabinet * pCabinet = 0;

  try
  {
    pCabinet = pDecompressor->open(pDecompressor, const_cast<char *>(cabinetPath.Get()));

    if (pCabinet == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The cabinet file could not be opened."), "path", cabinetPath.ToString());
    }

    size_t prefixLen = prefix.length();

    unsigned fileCount = 0;

    for (mscabd_file * pCabFile = pCabinet->files; pCabFile != nullptr; pCabFile = pCabFile->next)
    {
      PathName dest(pCabFile->filename);

#if defined(MIKTEX_UNIX)
      dest.ToUnix();
#endif

      // skip directory prefix
      if (PathName::Compare(prefix, dest, prefixLen) == 0)
      {
        PathName tmp(dest);
        dest = tmp.Get() + prefixLen;
      }

      // make the destination path name
      PathName path(destDir);
      if (!makeDirectories)
      {
        dest.RemoveDirectorySpec();
      }
      path /= dest;

      // notify the client
      if (pCallback != nullptr)
      {
        pCallback->OnBeginFileExtraction(path.ToString(), pCabFile->length);
      }

      // create the destination directory
      Directory::Create(PathName(path).RemoveFileSpec());

      // remove the existing file
      if (File::Exists(path))
      {
        File::Delete(path, { FileDeleteOption::TryHard });
      }

      // extract the file
      int r = pDecompressor->extract(pDecompressor, pCabFile, path.GetData());
      if (r != MSPACK_ERR_OK)
      {
         MIKTEX_FATAL_ERROR_2(T_("The member could not bex extracted from the cabinet file."), "cabinetPath", cabinetPath.ToString(), "member", pCabFile->filename, "ret", std::to_string(r));
      }

      fileCount += 1;

      // set time when the file was created
      struct tm tm;
      tm.tm_sec = pCabFile->time_s;
      tm.tm_min = pCabFile->time_m;
      tm.tm_hour = pCabFile->time_h;
      tm.tm_mday = pCabFile->date_d;
      tm.tm_mon = pCabFile->date_m - 1;
      tm.tm_year = pCabFile->date_y - 1900;
      tm.tm_isdst = 0;
      time_t time = mktime(&tm);
      if (time == static_cast<time_t>(-1))
      {
        MIKTEX_FATAL_CRT_ERROR("mktime");
      }
      File::SetTimes(path, time, time, time);

      // set file attributes
      SetAttributes(path, pCabFile->attribs);

      // notify the client
      if (pCallback != nullptr)
      {
        pCallback->OnEndFileExtraction("", pCabFile->length);
      }
    }

    traceStream->WriteFormattedLine("libextractor", T_("extracted %u file(s)"), fileCount);

    pDecompressor->close(pDecompressor, pCabinet);
    pCabinet = nullptr;

  }
  catch (const exception &)
  {
    if (pCabinet != nullptr)
    {
      pDecompressor->close(pDecompressor, pCabinet);
    }
    throw;
  }
}

void CabExtractor::Extract(Stream * pStream, const PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const string & prefix)
{
  UNUSED_ALWAYS(pStream);
  UNUSED_ALWAYS(destDir);
  UNUSED_ALWAYS(makeDirectories);
  UNUSED_ALWAYS(pCallback);
  UNUSED_ALWAYS(prefix);
  UNIMPLEMENTED();
}
