/* TarLzmaExtractor.cpp:

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

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <memory>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/LzmaStream>
#include <miktex/Trace/StopWatch>

#include "internal.h"

#include "TarLzmaExtractor.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Trace;
using namespace std;

void TarLzmaExtractor::Extract(const PathName& path, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(traceStopWatch.get(), TRACE_FACILITY, path.GetFileName().ToString());
  traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracting {0}"), Q_(path)));
  unique_ptr<LzmaStream> lzmaStream = LzmaStream::Create(path, true);
  TarExtractor::Extract(lzmaStream.get(), destDir, makeDirectories, callback, prefix);
}
