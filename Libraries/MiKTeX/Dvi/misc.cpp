/* misc.cpp: common DVI stuff

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
struct UnmapViewOfFile_
{
public:
  void operator() (void * p) const
  {
    if (!UnmapViewOfFile(p))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("UnmapViewOfFile");
    }
  }
};

typedef AutoResource<void *, UnmapViewOfFile_> AutoUnmapViewOfFile;
#endif

#if defined(MIKTEX_WINDOWS)
struct DeleteMetaFile_
{
public:
  void operator() (HMETAFILE hMetaFile) const
  {
    if (!DeleteMetaFile(hMetaFile))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("DeleteMetaFile");
    }
  }
};

typedef AutoResource<HMETAFILE, DeleteMetaFile_> AutoDeleteMetaFile;
#endif

InputStream::InputStream(const char * lpszFileName) :
  fileName(lpszFileName),
  stream(File::Open(lpszFileName, FileMode::Open, FileAccess::Read, false))
{
}

InputStream::InputStream(const BYTE * pBytes, size_t nBytes)
{
  this->pBytes = new char[nBytes];
  this->nBytes = nBytes;
  memcpy(this->pBytes, pBytes, nBytes);
  this->idx = 0;
}

InputStream::~InputStream()
{
  try
  {
    if (pBytes != nullptr)
    {
      delete[] pBytes;
      nBytes = 0;
      pBytes = nullptr;
      idx = 0;
    }
    else
    {
      stream.Close();
    }
  }
  catch (const exception &)
  {
  }
}

bool InputStream::Read(void * pBytes, size_t nBytes, bool allowEof)
{
  if (this->pBytes != nullptr)
  {
    if (IsEndOfStream())
    {
      if (!allowEof)
      {
        MIKTEX_UNEXPECTED();
      }
      else
      {
        return false;
      }
    }
    else if (idx + nBytes > this->nBytes)
    {
      MIKTEX_UNEXPECTED();
    }
    else
    {
      memcpy(pBytes, this->pBytes + idx, nBytes);
      idx += nBytes;
      return true;
    }
  }
  else
  {
    if (stream.Read(pBytes, nBytes) != nBytes)
    {
      if (!allowEof)
      {
        MIKTEX_UNEXPECTED();
      }
      return false;
    }
    else
    {
      return true;
    }
  }
}

void InputStream::SkipBytes(long count)
{
  if (pBytes != nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  else
  {
    stream.Seek(count, SeekOrigin::Current);
  }
}

void InputStream::SetReadPosition(long offset, SeekOrigin origin)
{
  if (pBytes != nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  else
  {
    stream.Seek(offset, origin);
  }
}

long InputStream::GetReadPosition()
{
  if (pBytes != nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  else
  {
    return stream.GetPosition();
  }
}

bool InputStream::TryToReadByte(int & by)
{
  if (!Read(reinterpret_cast<char*>(buffer), 1, true))
  {
    return false;
  }
  else
  {
    by = (buffer[0] & 0xff);
    return true;
  }
}

DviException::DviException() :
  MiKTeXException(T_("DVI exception."))
{
}

DviException::DviException(const string & programInvocationName, const string & message, const KVMAP & info, const SourceLocation & sourceLocation) :
  MiKTeXException(programInvocationName, message, info, sourceLocation)
{
}

DviFileInUseException::DviFileInUseException()
{
}

DviFileInUseException::DviFileInUseException (const string & programInvocationName, const string & message, const KVMAP & info, const SourceLocation & sourceLocation) :
  DviException(programInvocationName, message, info, sourceLocation)
{
}

DviPageNotFoundException::DviPageNotFoundException()
{
}

DviPageNotFoundException::DviPageNotFoundException(const string & programInvocationName, const string & message, const KVMAP & info, const SourceLocation & sourceLocation) :
  DviException(programInvocationName, message, info, sourceLocation)
{
}

int ScaleFix(int tfm, int z)
{
  int alpha, beta;      // quantities used in the scaling computation

  // replace z by z' and compute alpha/beta
  alpha = 16;
  while (z >= 040000000)
  {
    z /= 2;
    alpha += alpha;
  }
  beta = 256 / alpha;
  alpha *= z;

  // Convert the width value
  int b0 = (tfm >> 24) & 0xff;
  int b1 = (tfm >> 16) & 0xff;
  int b2 = (tfm >> 8) & 0xff;
  int b3 = tfm & 0xff;

  MIKTEX_ASSERT(beta != 0);

  tfm = (((((b3 * z) / 0400) + (b2 * z)) / 0400) + (b1 * z)) / beta;

  if (b0 == 255)
  {
    tfm -= alpha;
  }

  return tfm;
}

GraphicsInclusion::~GraphicsInclusion()
{
}

// Borrowed from the mfedit sample.
HENHMETAFILE GraphicsInclusionImpl::LoadEnhMetaFile(const PathName & fileName)
{
  HENHMETAFILE hEmf = nullptr;

  const DWORD META32_SIGNATURE = 0x464D4520;      // ' EMF'
  const DWORD ALDUS_ID = 0x9AC6CDD7;
  const size_t APMSIZE = 22;

  HANDLE hFile = CreateFileW(UW_(fileName.Get()), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "fileName", fileName.ToString())
  }

  AutoHANDLE autoCloseFile(hFile);

  HANDLE hMapFile = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, L"MIKEMF");

  if (hMapFile == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileMappingW", "fileName", fileName.Get());
  }

  AutoHANDLE autoCloseFileMapping(hMapFile);

  void * pMapFile = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);

  if (pMapFile == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("MapViewOfFile", "fileName", fileName.Get());
  }

  AutoUnmapViewOfFile autoUnmap(pMapFile);

  LPENHMETAHEADER pEmh = reinterpret_cast<LPENHMETAHEADER>(pMapFile);

  if (pEmh->dSignature == META32_SIGNATURE)
  {
    hEmf = GetEnhMetaFileW(UW_(fileName.Get()));
    if (hEmf == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
    }
    return hEmf;
  }

  if (*reinterpret_cast<LPDWORD>(pEmh) == ALDUS_ID)
  {
    DWORD size = *reinterpret_cast<LPDWORD>(reinterpret_cast<PBYTE>(pMapFile) + APMSIZE + 6);
    // FIXME: use device context
    hEmf = SetWinMetaFileBits(size * 2, reinterpret_cast<PBYTE>(pMapFile) + APMSIZE, nullptr, nullptr);
    if (hEmf == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
    }
    return hEmf;
  }

  HMETAFILE hMf = GetMetaFileW(UW_(fileName.Get()));

  if (hMf == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
  }

  AutoDeleteMetaFile autoDeleteMetaFile(hMf);

  UINT size = GetMetaFileBitsEx(hMf, 0, nullptr);

  if (size == 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
  }

  void * pvData = malloc(size);

  if (pvData == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }

  AutoMemoryPointer autoFree(pvData);

  size = GetMetaFileBitsEx(hMf, size, pvData);

  if (size == 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
  }

  hEmf = SetWinMetaFileBits(size, const_cast<const PBYTE>(reinterpret_cast<PBYTE>(pvData)), nullptr, nullptr);

  if (hEmf == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The metafile could not be loaded."), "path", fileName.ToString());
  }

  return hEmf;
}
