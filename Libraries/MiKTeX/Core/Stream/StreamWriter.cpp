/* StreamWriter.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/StreamWriter.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

StreamWriter::StreamWriter(const PathName & path)
  : stream(File::Open(path, FileMode::Create, FileAccess::Write))
{
}

StreamWriter::~StreamWriter()
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void StreamWriter::Close()
{
  stream.Close();
}

inline int FPutC(int ch, FILE * stream)
{
  int chWritten = fputc(ch, stream);
  if (chWritten == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("fputc");
  }
  return chWritten;
}

inline void FPutS(const char * lpsz, FILE * stream)
{
  int ok = fputs(lpsz, stream);
  if (ok < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fputs");
  }
}

void StreamWriter::Write(char ch)
{
  FPutC(ch, stream.Get());
}

void StreamWriter::Write(const string & line)
{
  FPutS(line.c_str(), stream.Get());
}

void StreamWriter::WriteLine(const string & line)
{
  Write(line);
  WriteLine();
}

void StreamWriter::WriteLine()
{
  FPutC('\n', stream.Get());
}

void StreamWriter::WriteFormatted(const char * lpszFormat, ...)
{
  va_list marker;
  va_start(marker, lpszFormat);
  Write(StringUtil::FormatString(lpszFormat, marker));
  va_end(marker);
}

void StreamWriter::WriteFormattedLine(const char * lpszFormat, ...)
{
  va_list marker;
  va_start(marker, lpszFormat);
  WriteLine(StringUtil::FormatString(lpszFormat, marker));
  va_end(marker);
}
