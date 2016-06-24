/* PkFont.cpp:

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

   /* some tricks are borrowed from dvips, gftopk and other celebrities */

#include "StdAfx.h"
#include "internal.h"

PkFont::PkFont(DviImpl * pDvi, int checkSum, int scaledSize, int designSize, const char * lpszAreaName, const char * lpszFontName, const char * lpszFileName, double tfmConv, double conv, int mag, const char * lpszMetafontMode, int baseDpi) :
  DviFont(pDvi, checkSum, scaledSize, designSize, lpszAreaName, lpszFontName, lpszFileName, tfmConv, conv),
  mag(mag),
  metafontMode(lpszMetafontMode),
  baseDpi(baseDpi),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  trace_pkfont(TraceStream::Open(MIKTEX_TRACE_DVIPKFONT))
{
  trace_pkfont->WriteFormattedLine("libdvi", T_("creating pk font object '%s'"), dviInfo.name.c_str());
  for (int i = 0; i < 30; ++i)
  {
    existSizes[i] = 0;
  }
  existSizes[1] = 99999999;
  dviInfo.isVirtualFont = false;
}

PkFont::~PkFont()
{
  try
  {
    for (MAPNUMTOPKCHAR::iterator it = pkChars.begin(); it != pkChars.end(); ++it)
    {
      delete it->second;
      it->second = nullptr;
    }
    pkChars.clear();
    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }
    if (trace_pkfont != nullptr)
    {
      trace_pkfont->Close();
      trace_pkfont = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

void PkFont::AddSize(int rhsize)
{
  int hsize = rhsize;
  int * p;
  for (p = existSizes; *p < hsize; ++p)
  {
    ;
  }
  if (*p == hsize)
  {
    return;
  }
  do
  {
    int t = *p;
    *p++ = hsize;
    hsize = t;
  } while (hsize);
}

void PkFont::AddDpi(int hsize)
{
  AddSize(hsize);
  AddSize((hsize * 116161L + 53020L) / 106040L);
  int a = hsize;
  int b = 0;
  int c = 1;
  for (int i = 0; i < 9; ++i)
  {
    b = 6 * b + (a % 5) * c;
    a = a + a / 5;
    c = c * 5;
    if (b > c)
    {
      b -= c;
      ++a;
    }
    if (b + b >= c)
    {
      AddSize(a + 1);
    }
    else
    {
      AddSize(a);
    }
  }
}

int PkFont::CheckDpi(int dpi, int baseDpi)
{
  if (!checkDpi)
  {
    AddDpi(baseDpi);
    checkDpi = true;
  }
  int i;
  for (i = 0; existSizes[i] < dpi; ++i)
  {
    ;
  }
  int margin = 1 + dpi / 500;
  if (existSizes[i] - dpi <= margin)
  {
    return existSizes[i];
  }
  else if (dpi - existSizes[i - 1] <= margin)
  {
    return existSizes[i - 1];
  }
  else
  {
    return dpi;
  }
}

const int pk_id = 89;
const int pk_xxx1 = 240;
const int pk_xxx2 = 241;
const int pk_xxx3 = 242;
const int pk_xxx4 = 243;
const int pk_yyy = 244;
const int pk_post = 245;
const int pk_no_op = 246;
const int pk_pre = 247;

void PkFont::Read()
{
  if (!pkChars.empty() || dviInfo.notLoadable)
  {
    return;
  }

  trace_pkfont->WriteFormattedLine("libdvi", T_("going to load pk font %s"), dviInfo.name.c_str());

  int dpi =
    static_cast<int>((static_cast<double>(mag)
      * static_cast<double>(scaledAt)
      * static_cast<double>(baseDpi))
      / (static_cast<double>(designSize) * 1000.0)
      + 0.5);
  dpi = CheckDpi(dpi, baseDpi);

  dviInfo.notLoadable = true;

  PathName fileName;

  bool fontFileExists = session->FindPkFile(dviInfo.name.c_str(), metafontMode.c_str(), dpi, fileName);

  if (!fontFileExists)
  {
    if (Make(dviInfo.name, dpi, baseDpi, metafontMode))
    {
      fontFileExists =
        session->FindPkFile(dviInfo.name.c_str(), metafontMode.c_str(), dpi, fileName);
      if (!fontFileExists)
      {
        // this shouldn't happen; but it does (#521481)
      }
    }
    if (!fontFileExists)
    {
      dviInfo.transcript += "\r\n";
      dviInfo.transcript += T_("Loading 'cmr10' instead.\r\n");
      trace_error->WriteFormattedLine("libdvi", T_("'%s' not loadable - loading 'cmr10' instead!"), dviInfo.name.c_str());
      if (!(session->FindPkFile("cmr10", metafontMode.c_str(), dpi, fileName)
        || (Make("cmr10", dpi, baseDpi, metafontMode)
          && session->FindPkFile("cmr10", metafontMode.c_str(), dpi, fileName))))
      {
        dviInfo.transcript += T_("'cmr10' not loadable either!");
        trace_error->WriteLine("libdvi", T_("'cmr10' not loadable - will display blank chars!"));
        return;
      }
    }
  }

  dviInfo.fileName = fileName.ToString();

  trace_pkfont->WriteFormattedLine("libdvi", T_("opening pk file %s"), fileName.Get());

  InputStream inputstream(fileName.Get());
  int b;
  while (inputstream.TryToReadByte(b))
  {
    // process a PK command
    switch (b)
    {
    case pk_xxx1:
    case pk_xxx2:
    case pk_xxx3:
    case pk_xxx4:
    {
      int k;
      int n = b - pk_xxx1;
      switch (n)
      {
      case 0:
        k = inputstream.ReadByte();
        break;
      case 1:
        k = inputstream.ReadPair();
        break;
      case 2:
        k = inputstream.ReadTrio();
        break;
      case 3:
        k = inputstream.ReadSignedQuad();
        break;
      default:
        MIKTEX_ASSERT(false);
        __assume (false);
      }
      inputstream.SkipBytes(k);
    }
    break;

    case pk_yyy:
      inputstream.SkipBytes(4); // ignore value
      break;

    case pk_post:
      break;

    case pk_no_op:
      break;

    case pk_pre:
    {
      if (inputstream.ReadByte() != pk_id)
      {
        FATAL_DVI_ERROR_2(T_("Bad font file."), "fileName", dviInfo.name.c_str());
      }
      int len = inputstream.ReadByte();
      char tmp[256];
      inputstream.Read(tmp, len);
      tmp[len] = 0;
      dviInfo.comment = tmp;
      int my_designSize = inputstream.ReadSignedQuad();
      int my_checkSum = inputstream.ReadSignedQuad();
      hppp = inputstream.ReadSignedQuad();
      vppp = inputstream.ReadSignedQuad();
      trace_pkfont->WriteFormattedLine("libdvi", "comment: %s", dviInfo.comment.c_str());
      trace_pkfont->WriteFormattedLine("libdvi", "designSize: %d", my_designSize);
      trace_pkfont->WriteFormattedLine("libdvi", "checkSum: 0%o", my_checkSum);
      trace_pkfont->WriteFormattedLine("libdvi", "hppp: %d", hppp);
      trace_pkfont->WriteFormattedLine("libdvi", "vppp: %d", vppp);

      if (my_designSize * tfmConv != designSize)
      {
        trace_error->WriteFormattedLine("libdvi", T_("%s: designSize mismatch"), dviInfo.name.c_str());
      }
      if (my_checkSum != checkSum)
      {
        trace_error->WriteFormattedLine("libdvi", T_("%s: checkSum mismatch"), dviInfo.name.c_str());
      }
    }
    break;

    default:

      // do a character definition
      PkChar * pPkChar = new PkChar(this);
      pPkChar->Read(inputstream, b);
      pkChars[pPkChar->GetCharacterCode()] = pPkChar;
      break;
    }
  }

  dviInfo.notLoadable = !fontFileExists;
}

bool PkFont::Make(const string & name, int dpi, int baseDpi, const string & metafontMode)
{
  dviInfo.transcript += "\r\n";
  dviInfo.transcript += T_("Making PK font:\r\n");
  PathName pathMakePk;
  string commandLine = session->MakeMakePkCommandLine(name.c_str(), dpi, baseDpi, metafontMode.c_str(), pathMakePk, TriState::Undetermined);
  dviInfo.transcript += commandLine;
  dviInfo.transcript += "\r\n";
  pDviImpl->Progress(DviNotification::BeginLoadFont, "%s...", dviInfo.name.c_str());
  ProcessOutput<4096> makepkOutput;
  int exitCode;
  bool b = Process::Run(pathMakePk, commandLine.c_str(), &makepkOutput, &exitCode, nullptr) && exitCode == 0;
  if (!b)
  {
    trace_error->WriteLine("libdvi", makepkOutput.StdoutToString());
  }
  dviInfo.transcript += makepkOutput.StdoutToString();
  dviInfo.transcript += "\r\n";
  return b;
}

PkChar * PkFont::operator[] (unsigned long idx)
{
  Read();
  PkChar * pPkChar = pkChars[idx];
  if (pPkChar == nullptr)
  {
    trace_pkfont->WriteFormattedLine("libdvi", T_("%s: nil character at %u"), dviInfo.name.c_str(), idx);
    pPkChar = new PkChar(this);
    pkChars[idx] = pPkChar;
  }
  return pPkChar;
}

void PkFont::ReadTFM()
{
  if (!pkChars.empty() || dviInfo.notLoadable)
  {
    return;
  }

  trace_pkfont->WriteFormattedLine("libdvi", T_("going to load TFM file %s"), dviInfo.name.c_str());

  PathName fileName;

  bool tfmFileExists = session->FindTfmFile(dviInfo.name.c_str(), fileName, false);

  if (!tfmFileExists)
  {
    if (MakeTFM(dviInfo.name))
    {
      tfmFileExists = session->FindTfmFile(dviInfo.name.c_str(), fileName, false);
      if (!tfmFileExists)
      {
        // this shouldn't happen; but it does (#521481)
      }
    }
    if (!tfmFileExists)
    {
      dviInfo.transcript += "\r\n";
      dviInfo.transcript += T_("Loading 'cmr10' instead.\r\n");
      trace_error->WriteFormattedLine("libdvi", T_("'%s' not loadable - loading 'cmr10' instead!"), dviInfo.name.c_str());
      if (!(session->FindTfmFile("cmr10", fileName, false)
        || (MakeTFM("cmr10")
          && session->FindTfmFile("cmr10", fileName, false))))
      {
        dviInfo.transcript += T_("'cmr10' not loadable either!");
        trace_error->WriteLine("libdvi", T_("'cmr10' not loadable - will display blank chars!"));
        return;
      }
    }
  }
}

bool PkFont::MakeTFM(const string & name)
{
  dviInfo.transcript += "\r\n";
  dviInfo.transcript += T_("Making TFM file:\r\n");
  PathName makeTFM;
  if (!session->FindFile(MIKTEX_MAKETFM_EXE, FileType::EXE, makeTFM))
  {
    MIKTEX_UNEXPECTED();
  }
  char szBasename[BufferSizes::MaxPath];
  PathName::Split(name.c_str(), nullptr, 0, szBasename, BufferSizes::MaxPath, nullptr, 0);
  CommandLineBuilder commandLine;
  commandLine.AppendOption("-v");
  commandLine.AppendArgument(szBasename);
  dviInfo.transcript += commandLine.ToString();
  dviInfo.transcript += "\r\n";
  pDviImpl->Progress(DviNotification::BeginLoadFont, "%s...", dviInfo.name.c_str());
  ProcessOutput<4096> maketfmOutput;
  int exitCode;
  bool done = Process::Run(makeTFM, commandLine.ToString(), &maketfmOutput, &exitCode, nullptr) && exitCode == 0;
  if (!done)
  {
    trace_error->WriteLine("libdvi", maketfmOutput.StdoutToString());
  }
  dviInfo.transcript += maketfmOutput.StdoutToString();
  dviInfo.transcript += "\r\n";
  return done;
}

