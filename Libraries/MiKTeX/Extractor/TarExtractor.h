/* TarExtractor.h:                                      -*- C++ -*-

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

#if !defined(BF702FE409EC4B9592640F4FD967F75B)
#define BF702FE409EC4B9592640F4FD967F75B

#include <miktex/Trace/TraceStream>

#include "miktex/Extractor/Extractor"

BEGIN_INTERNAL_NAMESPACE;

class TarExtractor : public MiKTeX::Extractor::Extractor
{
public:
  TarExtractor();

public:
  MIKTEXTHISCALL ~TarExtractor() override;

public:
  void MIKTEXTHISCALL Extract(const MiKTeX::Core::PathName& path, const MiKTeX::Core::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) override;

public:
  void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Core::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) override;

protected:
  size_t Read(void* data, size_t numBytes)
  {
    size_t n = streamIn->Read(data, numBytes);
    totalBytesRead += n;
    return n;
  }

protected:
  void ReadBlock(void* data);

protected:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

protected:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStopWatch;

protected:
  bool haveLongName;

protected:
  MiKTeX::Core::PathName longName;

protected:
  size_t totalBytesRead;

protected:
  MiKTeX::Core::Stream* streamIn = nullptr;

protected:
  void Skip(size_t bytes);

};

END_INTERNAL_NAMESPACE;

#endif
