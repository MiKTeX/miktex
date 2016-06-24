/* CabExtractor.h:                                      -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(AE5923232DF04F7888B2DD7F583253A4)
#define AE5923232DF04F7888B2DD7F583253A4

BEGIN_INTERNAL_NAMESPACE;

class CabExtractor : public MiKTeX::Extractor::Extractor
{
public:
  CabExtractor();

public:
  virtual MIKTEXTHISCALL ~CabExtractor();

public:
  virtual void MIKTEXTHISCALL Extract(const MiKTeX::Core::PathName & cabinetPath, const MiKTeX::Core::PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const std::string & str);

public:
  virtual void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream * pStream, const MiKTeX::Core::PathName & destDir, bool makeDirectories, IExtractCallback * pCallback, const std::string & str);

private:
  mscab_decompressor * pDecompressor = nullptr;

private:
  static struct mspack_file * Open(struct mspack_system * self, const char * lpszFileName, int mode);

private:
  static void Close(struct mspack_file * pFile);

private:
  static int Read(struct mspack_file * pFile, void * pBuffer, int numBytes);

private:
  static int Write(struct mspack_file * pFile, void * pBuffer, int numBytes);

private:
  static int Seek(struct mspack_file * pFile, off_t offset, int mode);

private:
  static off_t Tell(struct mspack_file * pFile);

private:
  static void Message(struct mspack_file * pFile, const char * lpszFormat, ...);

private:
  static void * Alloc(struct mspack_system * self, size_t numBytes);

private:
  static void Free(void * pv);

private:
  static void Copy(void * pSource, void * pDest, size_t numBytes);

private:
  struct MySystem : public mspack_system
  {
    CabExtractor * pCabExtractor = nullptr;
  };

private:
  struct MyFile
  {
    std::string fileName;
    FILE * pFile = nullptr;
  };

private:
  mspack_system mspackSystem;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;
};

END_INTERNAL_NAMESPACE;

#endif
