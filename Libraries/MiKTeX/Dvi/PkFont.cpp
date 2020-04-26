/* PkFont.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Paths>

#include "internal.h"

PkFont::PkFont(DviImpl* dviImpl, int checkSum, int scaledSize, int designSize, const char* area, const char* fontName, const char* fileName, double tfmConv, double conv, int mag, const char* metafontMode, int baseDpi) :
  DviFont(dviImpl, checkSum, scaledSize, designSize, area, fontName, fileName, tfmConv, conv),
  mag(mag),
  metafontMode(metafontMode),
  baseDpi(baseDpi),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, dviImpl->GetTraceCallback())),
  trace_pkfont(TraceStream::Open(MIKTEX_TRACE_DVIPKFONT, dviImpl->GetTraceCallback()))
{
  trace_pkfont->WriteLine("libdvi", fmt::format(T_("creating pk font object '{0}'"), dviInfo.name));
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
  catch (const exception&)
  {
  }
}

void PkFont::AddSize(int rhsize)
{
  int hsize = rhsize;
  int* p;
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

  trace_pkfont->WriteLine("libdvi", fmt::format(T_("going to load pk font {0}"), dviInfo.name));

  int dpi =
    static_cast<int>((static_cast<double>(mag)
      * static_cast<double>(scaledAt)
      * static_cast<double>(baseDpi))
      / (static_cast<double>(designSize) * 1000.0)
      + 0.5);
  dpi = CheckDpi(dpi, baseDpi);

  dviInfo.notLoadable = true;

  PathName fileName;

  bool fontFileExists = session->FindPkFile(dviInfo.name, metafontMode, dpi, fileName);

  if (!fontFileExists)
  {
    if (Make(dviInfo.name, dpi, baseDpi, metafontMode))
    {
      fontFileExists = session->FindPkFile(dviInfo.name, metafontMode, dpi, fileName);
      if (!fontFileExists)
      {
        // this shouldn't happen; but it does (#521481)
      }
    }
    if (!fontFileExists)
    {
      dviInfo.transcript += "\r\n";
      dviInfo.transcript += T_("Loading 'cmr10' instead.\r\n");
      trace_error->WriteLine("libdvi", fmt::format(T_("'{0}' not loadable - loading 'cmr10' instead!"), dviInfo.name));
      if (!(session->FindPkFile("cmr10", metafontMode, dpi, fileName)
        || (Make("cmr10", dpi, baseDpi, metafontMode)
          && session->FindPkFile("cmr10", metafontMode, dpi, fileName))))
      {
        dviInfo.transcript += T_("'cmr10' not loadable either!");
        trace_error->WriteLine("libdvi", T_("'cmr10' not loadable - will display blank chars!"));
        return;
      }
    }
  }

  dviInfo.fileName = fileName.ToString();

  trace_pkfont->WriteLine("libdvi", fmt::format(T_("opening pk file {0}"), fileName));

  InputStream inputstream(fileName.GetData());
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
      trace_pkfont->WriteLine("libdvi", fmt::format("comment: {0}", dviInfo.comment));
      trace_pkfont->WriteLine("libdvi", fmt::format("designSize: {0}", my_designSize));
      trace_pkfont->WriteLine("libdvi", fmt::format("checkSum: {0:o}", my_checkSum));
      trace_pkfont->WriteLine("libdvi", fmt::format("hppp: {0}", hppp));
      trace_pkfont->WriteLine("libdvi", fmt::format("vppp: {0}", vppp));

      if (my_designSize * tfmConv != designSize)
      {
        trace_error->WriteLine("libdvi", fmt::format(T_("{0}: designSize mismatch"), dviInfo.name));
      }
      if (my_checkSum != checkSum)
      {
        trace_error->WriteLine("libdvi", fmt::format(T_("{0}: checkSum mismatch"), dviInfo.name));
      }
    }
    break;

    default:

      // do a character definition
      PkChar* pkChar = new PkChar(this);
      pkChar->Read(inputstream, b);
      pkChars[pkChar->GetCharacterCode()] = pkChar;
      break;
    }
  }

  dviInfo.notLoadable = !fontFileExists;
}

bool PkFont::Make(const string& name, int dpi, int baseDpi, const string& metafontMode)
{
  dviInfo.transcript += "\r\n";
  dviInfo.transcript += T_("Making PK font:\r\n");
  PathName pathMakePk;
  vector<string> args = session->MakeMakePkCommandLine(name, dpi, baseDpi, metafontMode, pathMakePk, TriState::Undetermined);
  dviInfo.transcript += CommandLineBuilder(args).ToString();
  dviInfo.transcript += "\r\n";
  dviImpl->Progress(DviNotification::BeginLoadFont, fmt::format("{0}...", dviInfo.name));
  ProcessOutput<4096> makepkOutput;
  int exitCode;
  bool b = Process::Run(pathMakePk, args, &makepkOutput, &exitCode, nullptr) && exitCode == 0;
  if (!b)
  {
    trace_error->WriteLine("libdvi", makepkOutput.StdoutToString());
  }
  dviInfo.transcript += makepkOutput.StdoutToString();
  dviInfo.transcript += "\r\n";
  return b;
}

PkChar* PkFont::operator[] (unsigned long idx)
{
  Read();
  PkChar* pkChar = pkChars[idx];
  if (pkChar == nullptr)
  {
    trace_pkfont->WriteLine("libdvi", fmt::format(T_("{0}: nil character at {1}"), dviInfo.name, idx));
    pkChar = new PkChar(this);
    pkChars[idx] = pkChar;
  }
  return pkChar;
}

void PkFont::ReadTFM()
{
  if (!pkChars.empty() || dviInfo.notLoadable)
  {
    return;
  }

  trace_pkfont->WriteLine("libdvi", fmt::format(T_("going to load TFM file {0}"), dviInfo.name));

  PathName fileName;

  bool tfmFileExists = session->FindTfmFile(dviInfo.name, fileName, false);

  if (!tfmFileExists)
  {
    if (MakeTFM(dviInfo.name))
    {
      tfmFileExists = session->FindTfmFile(dviInfo.name, fileName, false);
      if (!tfmFileExists)
      {
        // this shouldn't happen; but it does (#521481)
      }
    }
    if (!tfmFileExists)
    {
      dviInfo.transcript += "\r\n";
      dviInfo.transcript += T_("Loading 'cmr10' instead.\r\n");
      trace_error->WriteLine("libdvi", fmt::format(T_("'{0}' not loadable - loading 'cmr10' instead!"), dviInfo.name));
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

bool PkFont::MakeTFM(const string& name)
{
  dviInfo.transcript += "\r\n";
  dviInfo.transcript += T_("Making TFM file:\r\n");
  PathName makeTFM;
  if (!session->FindFile(MIKTEX_MAKETFM_EXE, FileType::EXE, makeTFM))
  {
    MIKTEX_UNEXPECTED();
  }
  PathName baseName = PathName(name).GetFileNameWithoutExtension();
  vector<string> args{ makeTFM.GetFileNameWithoutExtension().ToString() };
  args.push_back("-v");
  args.push_back(baseName.ToString());
  dviInfo.transcript += CommandLineBuilder(args).ToString();
  dviInfo.transcript += "\r\n";
  dviImpl->Progress(DviNotification::BeginLoadFont, fmt::format("{0}...", dviInfo.name));
  ProcessOutput<4096> maketfmOutput;
  int exitCode;
  bool done = Process::Run(makeTFM, args, &maketfmOutput, &exitCode, nullptr) && exitCode == 0;
  if (!done)
  {
    trace_error->WriteLine("libdvi", maketfmOutput.StdoutToString());
  }
  dviInfo.transcript += maketfmOutput.StdoutToString();
  dviInfo.transcript += "\r\n";
  return done;
}

