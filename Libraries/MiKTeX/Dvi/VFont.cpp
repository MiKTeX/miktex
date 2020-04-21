/* VFont.cpp:

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

#include "config.h"

#include <miktex/Core/Quoter>

#include "internal.h"

VFont::VFont(DviImpl* dviImpl, int checkSum, int scaledSize, int designSize, const char* area, const char* fontName, const char* fileName, double tfmConv, double conv, int mag, const char* metafontMode, int baseDpi) :
  DviFont(dviImpl, checkSum, scaledSize, designSize, area, fontName, fileName, tfmConv, conv),
  mag(mag),
  metafontMode(metafontMode),
  baseDpi(baseDpi),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, dviImpl->GetTraceCallback())),
  trace_vfont(TraceStream::Open(MIKTEX_TRACE_DVIVFONT, dviImpl->GetTraceCallback()))
{
  dviInfo.isVirtualFont = true;
}

VFont::~VFont()
{
  try
  {
    for (MAPNUMTOVFCHAR::iterator  it1 = characterTable.begin(); it1 != characterTable.end(); ++it1)
    {
      delete it1->second;
      it1->second = nullptr;
    }
    characterTable.clear();
    for (FontMap::iterator it2 = fontMap.begin(); it2 != fontMap.end(); ++it2)
    {
      delete it2->second;
      it2->second = nullptr;
    }
    fontMap.clear();
    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }
    if (trace_vfont != nullptr)
    {
      trace_vfont->Close();
      trace_vfont = nullptr;
    }
  }
  catch (const exception&)
  {
  }
}

const short pre = 247;
const short id_byte = 202;
const short post = 248;
const short long_char = 242;
const short fnt_def1 = 243;
const short fnt_def2 = 244;
const short fnt_def3 = 245;
const short fnt_def4 = 246;

void VFont::Read()
{
  if (!characterTable.empty() || dviInfo.notLoadable)
  {
    return;
  }

  dviInfo.notLoadable = true;

  InputStream stream(dviInfo.fileName.c_str());

  trace_vfont->WriteFormattedLine("libdvi", T_("reading vf file %s"), Q_(dviInfo.fileName));

  if (stream.ReadByte() != pre)
  {
    FATAL_DVI_ERROR_2(T_("Not a VF file."), "fileName", dviInfo.fileName);
  }

  ReadPreamble(stream);
  ReadFontDefsAndCharPackets(stream);
  ReadPostamble(stream);

  dviInfo.notLoadable = false;
}

void VFont::ReadPreamble(InputStream& inputStream)
{
  if (inputStream.ReadByte() != id_byte)
  {
    FATAL_DVI_ERROR_2(T_("Bad VF file."), "fileName", dviInfo.fileName);
  }

  unsigned long commentSize = inputStream.ReadByte();

  char tmp[256];
  inputStream.Read(tmp, commentSize);
  tmp[commentSize] = 0;
  dviInfo.comment = tmp;

  int my_checkSum = inputStream.ReadSignedQuad();
  int my_designSize = inputStream.ReadSignedQuad();

  trace_vfont->WriteFormattedLine("libdvi", "comment: %s", dviInfo.comment.c_str());
  trace_vfont->WriteFormattedLine("libdvi", "checkSum: 0%o", my_checkSum);
  trace_vfont->WriteFormattedLine("libdvi", "designSize: %d", my_designSize);

  if (my_designSize * tfmConv != designSize)
  {
    trace_error->WriteFormattedLine("libdvi", T_("%s: designSize mismatch"), dviInfo.name.c_str());
  }

  if (my_checkSum != checkSum)
  {
    trace_error->WriteFormattedLine("libdvi", T_("%s: checkSum mismatch"), dviInfo.name.c_str());
  }
}

void VFont::ReadFontDefsAndCharPackets(InputStream& inputStream)
{
  short tempByte;
  do
  {
    tempByte = inputStream.ReadByte();
    if (tempByte != post)
    {
      if (tempByte > long_char)
      {
        ReadFontDef(inputStream, tempByte);
      }
      else
      {
        ReadCharPacket(inputStream, tempByte);
      }
    }
  } while (tempByte != post);
}

void VFont::ReadFontDef(InputStream& inputStream, short fntDefX)
{
  int fontNum;

  switch (fntDefX)
  {
  case fnt_def1:
    fontNum = inputStream.ReadByte();
    break;
  case fnt_def2:
    fontNum = inputStream.ReadPair();
    break;
  case fnt_def3:
    fontNum = inputStream.ReadTrio();
    break;
  case fnt_def4:
    fontNum = inputStream.ReadSignedQuad();
    break;
  default:
    FATAL_DVI_ERROR_2(T_("Bad VF file."), "fileName", dviInfo.fileName);
    break;
  }

  trace_vfont->WriteFormattedLine("libdvi", T_("defining font %d"), fontNum);

  int cs = inputStream.ReadSignedQuad();
  int ss = inputStream.ReadSignedQuad();
  int ds = inputStream.ReadSignedQuad();

  int areaNameSize = inputStream.ReadByte();
  int fontNameSize = inputStream.ReadByte();

  char szArea[256];
  inputStream.Read(szArea, areaNameSize);
  szArea[areaNameSize] = 0;

  char szName[256];
  inputStream.Read(szName, fontNameSize);
  szName[fontNameSize] = 0;

  trace_vfont->WriteFormattedLine("libdvi", "areaname: %s", szArea);
  trace_vfont->WriteFormattedLine("libdvi", "fontname: %s", szName);
  trace_vfont->WriteFormattedLine("libdvi", "checkSum: 0%o", cs);
  trace_vfont->WriteFormattedLine("libdvi", "scaledSize: %d", ss);
  trace_vfont->WriteFormattedLine("libdvi", "designSize: %d", ds);

  DviFont* pFont;
  PathName fileName;
  shared_ptr<Session> session = Session::Get();
  if (session->FindFile(szName, FileType::VF, fileName))
  {
    trace_vfont->WriteFormattedLine("libdvi", T_("found vf file %s"), Q_(fileName));
    pFont = new VFont(dviImpl, cs, ScaleFix(ss, scaledAt), static_cast<int>(ds * tfmConv), szArea, szName, fileName.GetData(), tfmConv, conv, mag, metafontMode.c_str(), baseDpi);
  }
  else if (dviImpl->GetPageMode() != DviPageMode::Dvips)
  {
    pFont = new PkFont(dviImpl, cs, ScaleFix(ss, scaledAt), static_cast<int>(ds * tfmConv), szArea, szName, "", tfmConv, conv, mag, metafontMode.c_str(), baseDpi);
  }
  else
  {
    pFont = new Tfm(dviImpl, cs, ScaleFix(ss, scaledAt), static_cast<int>(ds * tfmConv), szArea, szName, "", tfmConv, conv);
  }

  fontMap[fontNum] = pFont;
}

void VFont::ReadCharPacket(InputStream& inputStream, short size)
{
  VfChar* pVfChar = new VfChar(this);
  pVfChar->Read(inputStream, size, conv);
  characterTable[pVfChar->GetCharacterCode()] = pVfChar;
}

void VFont::ReadPostamble(InputStream& inputStream)

{
  int by;
  while (inputStream.TryToReadByte(by))
  {
    if (by != post)
    {
      FATAL_DVI_ERROR_2(T_("Bad VF file."), "fileName", dviInfo.fileName.c_str());
    }
  }
}

VfChar* VFont::GetCharAt(int idx)

{
  Read();
  return (characterTable[idx]);
}
