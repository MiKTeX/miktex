/* CabExtractor.h:                                      -*- C++ -*-

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

#pragma once

#if !defined(AE5923232DF04F7888B2DD7F583253A4)
#define AE5923232DF04F7888B2DD7F583253A4

#include <miktex/Trace/TraceStream>

#include <mspack.h>

BEGIN_INTERNAL_NAMESPACE;

class CabExtractor : public MiKTeX::Extractor::Extractor
{
public:
  CabExtractor();

public:
  MIKTEXTHISCALL ~CabExtractor() override;

public:
  void MIKTEXTHISCALL Extract(const MiKTeX::Core::PathName& path, const MiKTeX::Core::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& str) override;

public:
  void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Core::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& str) override;

private:
  mscab_decompressor* decompressor = nullptr;

private:
  static struct mspack_file* Open(struct mspack_system* self, const char* fileName, int mode);

private:
  static void Close(struct mspack_file* mspackFile);

private:
  static int Read(struct mspack_file* mspackFile, void* data, int numBytes);

private:
  static int Write(struct mspack_file* mspackFile, void* data, int numBytes);

private:
  static int Seek(struct mspack_file* mspackFile, off_t offset, int mode);

private:
  static off_t Tell(struct mspack_file* mspackFile);

private:
  static void Message(struct mspack_file* mspackFile, const char* lpszFormat, ...);

private:
  static void* Alloc(struct mspack_system* self, size_t numBytes);

private:
  static void Free(void* pv);

private:
  static void Copy(void* source, void* dest, size_t numBytes);

private:
  struct MySystem : public mspack_system
  {
    CabExtractor* pCabExtractor = nullptr;
  };

private:
  struct MyFile
  {
    std::string fileName;
    FILE* stdioFile = nullptr;
  };

private:
  mspack_system mspackSystem;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;
};

END_INTERNAL_NAMESPACE;

#endif
