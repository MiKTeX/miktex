/* PkFont.h:                                            -*- C++ -*-

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

typedef unordered_map<int, PkChar *> MAPNUMTOPKCHAR;

class PkFont : public DviFont
{
public:
  PkFont(DviImpl * pDvi, int checksum, int scaledSize, int designSize, const char * lpszAreaName, const char * lpszFontName, const char * lpszFileName, double tfmConf, double conv, int mag, const char * lpszMetafontMode, int baseDpi);

public:
  virtual ~PkFont();

public:
  PkChar * operator[] (unsigned long idx);

private:
  void AddSize(int rhsize);

private:
  void AddDpi(int hsize);

private:
  int CheckDpi(int dpi, int baseDpi);

private:
  bool Make(const string & name, int dpi, int baseDpi, const string & metafontMode);

private:
  bool MakeTFM(const string & name);

public:
  void Read();

public:
  void ReadTFM();

private:
  int mag;

private:
  string metafontMode;

private:
  int baseDpi;

private:
  MAPNUMTOPKCHAR pkChars;

private:
  int existSizes[30];

private:
  bool checkDpi = false;

private:
  int hppp;

private:
  int vppp;

private:
  unique_ptr<TraceStream> trace_error;

private:
  unique_ptr<TraceStream> trace_pkfont;

private:
  shared_ptr<Session> session = Session::Get();
};
