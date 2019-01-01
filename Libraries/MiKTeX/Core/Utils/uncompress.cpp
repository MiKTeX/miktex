/* uncompress.cpp: uncompressing files

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

#include <miktex/Core/BZip2Stream>
#include <miktex/Core/FileStream>
#include <miktex/Core/GzipStream>
#include <miktex/Core/LzmaStream>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

void Utils::UncompressFile(const PathName& pathIn, PathName& pathOut)
{
  SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("uncompressing %s..."), Q_(pathIn));
  if (!File::Exists(pathIn))
  {
    MIKTEX_FATAL_ERROR_2(T_("The file could not be found."), "path", pathIn.ToString());
  }
  PathName pathTempFileName;
  pathTempFileName.SetToTempFile();
  FileStream stream(File::Open(pathTempFileName, FileMode::Create, FileAccess::Write, false));
  unique_ptr<Stream> inputStream;
  if (pathIn.HasExtension(".gz"))
  {
    inputStream = GzipStream::Create(pathIn, true);
  }
  else if (pathIn.HasExtension(".bz2"))
  {
    inputStream = BZip2Stream::Create(pathIn, true);
  }
  else if (pathIn.HasExtension(".lzma") || pathIn.HasExtension(".xz"))
  {
    inputStream = LzmaStream::Create(pathIn, true);
  }
  else
  {
    MIKTEX_FATAL_ERROR_2(T_("Could not uncompress file."), "path", pathIn.ToString());
  }
  unsigned char buf[4096];
  size_t len;
  while ((len = inputStream->Read(buf, ARRAY_SIZE(buf))) > 0)
  {
    stream.Write(buf, len);
  }
  pathOut = pathTempFileName;
}
