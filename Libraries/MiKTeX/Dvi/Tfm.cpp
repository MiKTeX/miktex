/* Tfm.cpp:

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

   /* some tricks are borrowed from Dvips, TFtoPL and other celebrities */

#include "StdAfx.h"
#include "internal.h"

Tfm::Tfm(DviImpl * pDvi, int checkSum, int scaledSize, int designSize, const char * lpszAreaName, const char * lpszFontName, const char * lpszFileName, double tfmConv, double conv) :
  DviFont(pDvi, checkSum, scaledSize, designSize, lpszAreaName, lpszFontName, lpszFileName, tfmConv, conv),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  trace_tfm(TraceStream::Open(MIKTEX_TRACE_DVITFM))

{
  trace_tfm->WriteFormattedLine ("libdvi", T_("creating TFM object '%s'"), dviInfo.name.c_str());
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
  catch (const exception &)
  {
  }
}
DviChar * Tfm::operator[] (unsigned long idx)
{
  Read();
  DviChar * pDviChar = dviChars[idx];
  if (pDviChar == nullptr)
  {
    trace_tfm->WriteFormattedLine("libdvi", T_("%s: nil character at %u"), dviInfo.name.c_str(), idx);
    pDviChar = new DviChar(this);
    dviChars[idx] = pDviChar;
  }
  return pDviChar;
}

void Tfm::Read()
{
  if (!dviChars.empty() || dviInfo.notLoadable)
  {
    return;
  }

  trace_tfm->WriteFormattedLine("libdvi", T_("going to load TFM file %s"), dviInfo.name.c_str());

  PathName fileName;

  bool tfmFileExists = session->FindTfmFile(dviInfo.name.c_str(), fileName, false);

  if (!tfmFileExists)
  {
    if (Make(dviInfo.name))
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
      trace_error->WriteFormattedLine
        ("libdvi", T_("'%s' not loadable - loading 'cmr10' instead!"), dviInfo.name.c_str());
      if (!(session->FindTfmFile("cmr10", fileName, false)
        || (Make("cmr10")
          && session->FindTfmFile("cmr10", fileName, false))))
      {
        dviInfo.transcript += T_("'cmr10' not loadable either!");
        trace_error->WriteLine("libdvi", T_("'cmr10' not loadable - will display blank chars!"));
        return;
      }
    }
  }

  dviInfo.fileName = fileName.ToString();

  trace_tfm->WriteFormattedLine("libdvi", T_("opening TFM file %s"), fileName.Get());

  InputStream inputStream(fileName.Get());

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

  trace_tfm->WriteFormattedLine("libdvi", T_("header size: %ld"), lh);
  trace_tfm->WriteFormattedLine("libdvi", T_("smallest character code: %ld"), bc);
  trace_tfm->WriteFormattedLine("libdvi", T_("largest character code: %ld"), ec);
  trace_tfm->WriteFormattedLine("libdvi", T_("width table size: %ld"), nw);
  trace_tfm->WriteFormattedLine("libdvi", T_("height table size: %ld"), nh);
  trace_tfm->WriteFormattedLine("libdvi", T_("depth table size: %ld"), nd);
  trace_tfm->WriteFormattedLine("libdvi", T_("italic correction table size: %ld"), ni);
  trace_tfm->WriteFormattedLine("libdvi", T_("lig/kern table size: %ld"), nl);
  trace_tfm->WriteFormattedLine("libdvi", T_("kern table size: %ld"), nk);
  trace_tfm->WriteFormattedLine("libdvi", T_("extensible character table size: %ld"), ne);
  trace_tfm->WriteFormattedLine("libdvi", T_("font parameter size: %ld"), np);

  int my_checkSum = inputStream.ReadSignedQuad();

  trace_tfm->WriteFormattedLine("libdvi", "checkSum: 0%lo", my_checkSum);

  int my_designSize = inputStream.ReadSignedQuad();

  trace_tfm->WriteFormattedLine("libdvi", "designSize: %ld", my_designSize);

  if (my_checkSum != checkSum)
  {
    trace_error->WriteFormattedLine("libdvi", T_("%s: checkSum mismatch"), dviInfo.name.c_str());
  }

  if (my_designSize * tfmConv != designSize)
  {
    trace_error->WriteFormattedLine("libdvi", T_("%s: designSize mismatch"), dviInfo.name.c_str());
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
    DviChar * pDviChar = new DviChar(this);
    dviChars[charCode] = pDviChar;
    pDviChar->SetCharacterCode(charCode);
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

bool Tfm::Make(const string & name)
{
  dviInfo.transcript += "\r\n";
  dviInfo.transcript += T_("Making TFM file:\r\n");
  PathName makeTFM;
  if (!session->FindFile(MIKTEX_MAKETFM_EXE, FileType::EXE, makeTFM))
  {
    MIKTEX_UNEXPECTED();
  }
  char szBasename[BufferSizes::MaxPath];
  PathName::Split(name.c_str(), 0, 0, szBasename, BufferSizes::MaxPath, 0, 0);
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

