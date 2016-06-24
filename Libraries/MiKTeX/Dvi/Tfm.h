/* Tfm.h:                                               -*- C++ -*-

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

class DviChar;
class DviImpl;

typedef unordered_map<int, DviChar *> MAPNUMTODVICHAR;

class Tfm : public DviFont
{
public:
  Tfm(DviImpl * pDvi, int checksum, int scaledSize, int designSize, const char * lpszAreaName, const char * lpszFontName, const char * lpszFileName, double tfmConf, double conv);

public:
  virtual ~Tfm();

public:
  DviChar * operator[] (unsigned long idx);

private:
  bool Make(const string & name);

public:
  void Read();

private:
  MAPNUMTODVICHAR dviChars;

private:
  unique_ptr<TraceStream> trace_error;

private:
  unique_ptr<TraceStream> trace_tfm;

private:
  shared_ptr<Session> session = Session::Get();
};
