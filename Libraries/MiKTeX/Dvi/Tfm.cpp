/* Tfm.cpp:

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

   /* some tricks are borrowed from Dvips, TFtoPL and other celebrities */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Paths>

#include "internal.h"

Tfm::Tfm(DviImpl* dviImpl, int checkSum, int scaledSize, int designSize, const char* area, const char* fontName, const char* fileName, double tfmConv, double conv) :
  DviFont(dviImpl, checkSum, scaledSize, designSize, area, fontName, fileName, tfmConv, conv),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, dviImpl->GetTraceCallback())),
  trace_tfm(TraceStream::Open(MIKTEX_TRACE_DVITFM, dviImpl->GetTraceCallback()))

{
  trace_tfm->WriteLine("libdvi", fmt::format(T_("creating TFM object '{0}'"), dviInfo.name));
  dviInfo.isVirtualFont = false;
}

Tfm::~Tfm()
{
  try
  {
    for (MAPNUMTODVICHAR::iterator it = dviChars.begin(); it != dviChars.end(); ++it)
    {
      delete it->second;
      it->second = nullptr;
    }
    dviChars.clear();
    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }
    if (trace_tfm != nullptr)
    {
      trace_tfm->Close();
      trace_tfm = nullptr;
    }
  }
  catch (const exception&)
  {
  }
}
DviChar* Tfm::operator[](unsigned long idx)
{
  Read();
  DviChar* dviChar = dviChars[idx];
  if (dviChar == nullptr)
  {
    trace_tfm->WriteLine("libdvi", fmt::format(T_("{0}: nil character at %u"), dviInfo.name, idx));
    dviChar = new DviChar(this);
    dviChars[idx] = dviChar;
  }
  return dviChar;
}

void Tfm::Read()
{
  if (!dviChars.empty() || dviInfo.notLoadable)
  {
    return;
  }

  trace_tfm->WriteLine("libdvi", fmt::format(T_("going to load TFM file {0}"), dviInfo.name));

  PathName fileName;

  bool tfmFileExists = session->FindTfmFile(dviInfo.name, fileName, false);

  if (!tfmFileExists)
  {
    if (Make(dviInfo.name))
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
      if (!(session->FindTfmFile("cmr10", fileName, false) || (Make("cmr10") && session->FindTfmFile("cmr10", fileName, false))))
      {
        dviInfo.transcript += T_("'cmr10' not loadable either!");
        trace_error->WriteLine("libdvi", T_("'cmr10' not loadable - will display blank chars!"));
        return;
      }
    }
  }

  dviInfo.fileName = fileName.ToString();

  trace_tfm->WriteLine("libdvi", fmt::format(T_("opening TFM file {0}"), fileName));

  InputStream inputStream(fileName.GetData());

  long lf = inputStream.ReadSignedPair();

  if (lf == 0)
  {
    FATAL_DVI_ERROR(T_("Invalid TFM file."));
  }

  long lh = inputStream.ReadSignedPair();
  long bc = inputStream.ReadSignedPair();
  long ec = inputStream.ReadSignedPair();
  long nw = inputStream.ReadSignedPair();
  long nh = inputStream.ReadSignedPair();
  long nd = inputStream.ReadSignedPair();
  long ni = inputStream.ReadSignedPair();
  long nl = inputStream.ReadSignedPair();
  long nk = inputStream.ReadSignedPair();
  long ne = inputStream.ReadSignedPair();
  long np = inputStream.ReadSignedPair();

  trace_tfm->WriteLine("libdvi", fmt::format(T_("header size: %{0}"), lh));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("smallest character code: {0}"), bc));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("largest character code: {0}"), ec));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("width table size: {0}"), nw));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("height table size: {0}"), nh));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("depth table size: {0}"), nd));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("italic correction table size: {0}"), ni));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("lig/kern table size: {0}"), nl));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("kern table size: {0}"), nk));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("extensible character table size: {0}"), ne));
  trace_tfm->WriteLine("libdvi", fmt::format(T_("font parameter size: {0}"), np));

  int my_checkSum = inputStream.ReadSignedQuad();

  trace_tfm->WriteLine("libdvi", fmt::format("checkSum: {0:o}", my_checkSum));

  int my_designSize = inputStream.ReadSignedQuad();

  trace_tfm->WriteLine("libdvi", fmt::format("designSize: {0}", my_designSize));

  if (my_checkSum != checkSum)
  {
    trace_error->WriteLine("libdvi", fmt::format(T_("{0}: checkSum mismatch"), dviInfo.name));
  }

  if (my_designSize * tfmConv != designSize)
  {
    trace_error->WriteLine("libdvi", fmt::format(T_("{0}: designSize mismatch"), dviInfo.name));
  }

  inputStream.SkipBytes((lh - 2) * 4);

  struct TfmIndex
  {
    int widthIndex;
    int heightIndex;
    int depthIndex;
  };

  vector<TfmIndex> infoTable(ec);

  for (int charCode = bc; charCode < ec; ++charCode)
  {
    DviChar* dviChar = new DviChar(this);
    dviChars[charCode] = dviChar;
    dviChar->SetCharacterCode(charCode);
    TfmIndex tfmIndex;
    tfmIndex.widthIndex = inputStream.ReadSignedByte();
    int heightDepth = inputStream.ReadSignedByte();
    tfmIndex.heightIndex = ((heightDepth >> 4) & 15);
    tfmIndex.depthIndex = (heightDepth & 15);
    inputStream.SkipBytes(2);
    infoTable[charCode] = tfmIndex;
  }

  vector<int> widths(nw);

  for (int idx = 0; idx < nw; ++idx)
  {
    widths[idx] = inputStream.ReadSignedQuad();
  }

  vector<int> heights(nh);

  for (int idx = 0; idx < nh; ++idx)
  {
    heights[idx] = inputStream.ReadSignedQuad();
  }

  vector<int> depths(nd);

  for (int idx = 0; idx < nd; ++idx)
  {
    depths[idx] = inputStream.ReadSignedQuad();
  }

  // inputStream.Close ();

  for (int charCode = bc; charCode < ec; ++charCode)
  {
    int tfmWidth = ScaleFix(widths[infoTable[charCode].widthIndex], GetScaledAt());
    dviChars[charCode]->SetDviWidth(tfmWidth);
    int pixelWidth;
    if (tfmWidth >= 0)
    {
      pixelWidth = static_cast<int>(conv * tfmWidth + 0.5);
    }
    else
    {
      pixelWidth = -static_cast<int>(conv * -tfmWidth + 0.5);
    }
    dviChars[charCode]->SetWidth(pixelWidth);
  }
}

bool Tfm::Make(const string& name)
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
