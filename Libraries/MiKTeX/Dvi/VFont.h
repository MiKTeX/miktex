/* vfont.h:                                             -*- C++ -*-

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

#pragma once

class InputStream;
class VfChar;

typedef unordered_map<int, class VfChar *> MAPNUMTOVFCHAR;

class VFont : public DviFont
{
public:
  VFont(DviImpl * pDvi, int checkSum, int scaledSize, int designSize, const char * lpszAreaName, const char * lpszFontName, const char * lpszFileName, double tfmConv, double conv, int mag, const char * lpszMetafontMode, int baseDpi);

public:
  virtual ~VFont();

public:
  VfChar * GetCharAt(int idx);

public:
  const FontMap & GetFontMap() const
  {
    return fontMap;
  }

private:
  void Read();

private:
  void ReadPreamble(InputStream & inputStream);

private:
  void ReadFontDefsAndCharPackets(InputStream & inputStream);

private:
  void ReadFontDef(InputStream & inputStream, short fntDefX);

private:
  void ReadCharPacket(InputStream & inputStream, short size);

private:
  void ReadPostamble(InputStream & inputStream);

private:
  int mag;

private:
  string metafontMode;

private:
  int baseDpi;

private:
  int shrinkFactor;

private:
  MAPNUMTOVFCHAR characterTable;

private:
  FontMap fontMap;

private:
  unique_ptr<TraceStream> trace_error;

private:
  unique_ptr<TraceStream> trace_vfont;
};
