/* TarExtractor.cpp:

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

#include "TarExtractor.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

const size_t BLOCKSIZE = 512;

struct Header
{
private:
  int GetOctal(const char * lpszField) const
  {
#if defined(_MSC_VER)
#  define SSCANF sscanf_s
#else
#  define SSCANF sscanf
#endif

    int ret;

    MIKTEX_ASSERT_STRING(lpszField);

    if (lpszField[0] == 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (SSCANF(lpszField, "%o", &ret) != 1)
    {
      string info = "value=";
      info += lpszField;
      MIKTEX_UNEXPECTED();
    }

    return ret;

#undef SSCANF
  }

public:
  Header()
  {
    MIKTEX_ASSERT(sizeof(*this) == BLOCKSIZE);
  }

public:
  bool Check() const
  {
    unsigned myHeaderCheckSum = 0;
    size_t i;
    for (i = 0; i < offsetof(Header, chksum); ++i)
    {
      myHeaderCheckSum += reinterpret_cast<const unsigned char *>(this)[i];
    }
    myHeaderCheckSum += ' ' * sizeof(chksum);
    i += sizeof(chksum);
    for (; i < sizeof(*this); ++i)
    {
      myHeaderCheckSum += reinterpret_cast<const unsigned char *>(this)[i];
    }
    return myHeaderCheckSum == GetHeaderCheckSum();
  }

public:
  bool IsEndOfArchive() const
  {
    for (size_t i = 0; i < sizeof(*this); ++i)
    {
      if (reinterpret_cast<const unsigned char *>(this)[i] != 0)
      {
        return false;
      }
    }
    return true;
  }

private:
  char name[100];

public:
  PathName GetFileName() const
  {
    char namez[sizeof(name) + 1];
    memcpy(namez, name, sizeof(name));
    namez[sizeof(name)] = 0;
    PathName ret;
    if (IsUSTAR())
    {
      ret = prefix;
    }
    ret /= namez;
    return ret;
  }

private:
  char mode[8];

public:
  unsigned long GetFileMode() const
  {
    return GetOctal(mode);
  }

private:
  char uid[8];

private:
  char gid[8];

private:
  char size[12];

public:
  size_t GetFileSize() const
  {
    return GetOctal(size);
  }

private:
  char mtime[12];

public:
  time_t GetLastModificationTime() const
  {
    return GetOctal(mtime);
  }

private:
  char chksum[8];

public:
  unsigned long GetHeaderCheckSum() const
  {
    return GetOctal(chksum);
  }

public:
  enum Type
  {
    RegularFile = '0',
    AlternateRegularFile = '\0',
    Link = '1',
    SymbolicLink = '2',
    CharacterSpecial = '3',
    BlockSpecial = '4',
    Directory = '5',
    FIFOSpecial = '6',
    Reserved = '7',
    LongName = 'L'
  };

private:
  char typeflag[1];

public:
  Type GetType() const
  {
    return static_cast<Type>(typeflag[0]);
  }

public:
  bool IsNormalFile() const
  {
    return GetType() == RegularFile || GetType() == AlternateRegularFile;
  }

private:
  char linkname[100];

private:
  char magic[6];

public:
  bool IsUSTAR() const
  {
    return memcmp(magic, "ustar", 5) == 0 && (magic[5] == 0 || magic[5] == ' ');
  }

private:
  char version[2];

private:
  char uname[32];

private:
  char gname[32];

private:
  char devmajor[8];

private:
  char devminor[8];

private:
  char prefix[155];

private:
  char reservered[12];
};

void TarExtractor::ReadBlock(void * pBuffer)
{
  size_t n = pStreamIn->Read(pBuffer, BLOCKSIZE);
  totalBytesRead += n;
  if (n != BLOCKSIZE)
  {
    MIKTEX_UNEXPECTED();
  }
}

void TarExtractor::Skip(size_t numBytes)
{
  const size_t BUFSIZE = 4096;
  char buffer[BUFSIZE];
  size_t bytesRead = 0;
  while (bytesRead < numBytes)
  {
    size_t remaining = numBytes - bytesRead;
    size_t n = (remaining > BUFSIZE ? BUFSIZE : remaining);
    if (Read(buffer, n) != n)
    {
      MIKTEX_UNEXPECTED();
    }
    bytesRead += n;
  }
}

TarExtractor::TarExtractor() :
  traceStream(TraceStream::Open(MIKTEX_TRACE_EXTRACTOR)),
  haveLongName(false)
{
}

TarExtractor::~TarExtractor()
{
}

void TarExtractor::Extract(Stream * pStreamIn_, const PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const string & prefix)
{
  try
  {
    pStreamIn = pStreamIn_;
    totalBytesRead = 0;

    traceStream->WriteFormattedLine("libextractor", T_("extracting to %s (%s)"), Q_(destDir), (makeDirectories ? T_("make directories") : T_("don't make directories")));

    size_t len;
    Header header;
    size_t prefixLen = prefix.length();
    unsigned fileCount = 0;

    bool checkHeader = true;

    CharBuffer<char> buffer;
    buffer.Reserve(1024 * 1024);

    while ((len = Read(&header, sizeof(header))) > 0)
    {
      // read next header
      if (len != sizeof(header))
      {
        MIKTEX_UNEXPECTED();
      }

      if (header.IsEndOfArchive())
      {
        break;
      }

      if (checkHeader)
      {
        if (!header.Check())
        {
          MIKTEX_UNEXPECTED();
        }
#if ! defined(MIKTEX_DEBUG)
        checkHeader = false;
#endif
      }

      PathName dest = header.GetFileName();
      size_t size = header.GetFileSize();

      if (!header.IsNormalFile())
      {
        if (header.GetType() == Header::LongName)
        {
          if (size >= BLOCKSIZE)
          {
            MIKTEX_UNEXPECTED();
          }
          char longNameData[BLOCKSIZE];
          ReadBlock(longNameData);
          longNameData[size] = 0;
          longName = longNameData;
          haveLongName = true;
        }
        else
        {
          Skip(((size + sizeof(Header) - 1) / sizeof(Header)) * sizeof(Header));
        }
        continue;
      }

      if (haveLongName)
      {
        dest = longName;
        haveLongName = false;
      }

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
        pCallback->OnBeginFileExtraction(path.ToString(), size);
      }

      // create the destination directory
      Directory::Create(PathName(path).RemoveFileSpec());

      // remove the existing file
      if (File::Exists(path))
      {
        File::Delete(path, { FileDeleteOption::TryHard });
      }

      // extract the file
      FileStream streamOut(File::Open(path, FileMode::Create, FileAccess::Write, false));
      size_t bytesRead = 0;
      while (bytesRead < size)
      {
        size_t remaining = size - bytesRead;
        size_t n = (remaining > buffer.GetCapacity() ? buffer.GetCapacity() : remaining);
        if (Read(buffer.GetData(), n) != n)
        {
          MIKTEX_UNEXPECTED();
        }
        streamOut.Write(buffer.Get(), n);
        bytesRead += n;
      }
      streamOut.Close();

      // skip extra bytes
      if (bytesRead % sizeof(Header) > 0)
      {
        Skip(sizeof(Header) - bytesRead % sizeof(Header));
      }

      fileCount += 1;

      // set time when the file was created
      time_t time = header.GetLastModificationTime();
      File::SetTimes(path, time, time, time);

#if 0
      // set file attributes
      File::SetAttributes(path, todo);
#endif

      // notify the client
      if (pCallback != nullptr)
      {
        pCallback->OnEndFileExtraction("", size);
      }
    }

    traceStream->WriteFormattedLine("libextractor", T_("extracted %u file(s)"), fileCount);
  }
  catch (const exception &)
  {
    traceStream->WriteFormattedLine("libextractor", T_("%u bytes were read from the tar stream"), static_cast<unsigned>(totalBytesRead));
    throw;
  }
}

void TarExtractor::Extract(const PathName & tarPath, const PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const string & prefix)
{
  traceStream->WriteFormattedLine("libextractor", T_("extracting %s"), Q_(tarPath));
  FileStream stream(File::Open(tarPath, FileMode::Open, FileAccess::Read, false));
  Extract(&stream, destDir, makeDirectories, pCallback, prefix);
  stream.Close();
}
