/* DviFont.h:                                           -*- C++ -*-

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

class DviImpl;

class DviFont
{
protected:
  DviFont(DviImpl * pDviImpl, int checkSum, int scaledAt, int designSize, const char * lpszArea, const char * lpszName, const char * lpszFileName, double tfmConv, double conv);

public:
  virtual ~DviFont();

public:
  int GetInterWordSpacing();

public:
  int GetBackSpacing();

public:
  int GetLineSpacing();

public:
  const string & GetFileName()
  {
    return dviInfo.fileName;
  }

public:
  int GetScaledAt()
  {
    return scaledAt;
  }

public:
  inline int PixelShrink(int shrinkFactor, int pxl);

public:
  inline int WidthShrink(int shrinkFactor, int pxl);

public:
  DviImpl * GetDviObject()
  {
    return pDviImpl;
  }

public:
  bool IsNotLoadable()
  {
    return dviInfo.notLoadable;
  }

public:
  void GetInfo(DviFontInfo & fontinfo)
  {
    fontinfo = dviInfo;
  }

public:
  virtual void Read() = 0;

  // scaled size in DVI units
protected:
  int scaledAt;

  // design size in DVI units
protected:
  int designSize;

  // converts TFM units to DVI units
protected:
  double tfmConv;

  // converts DVI units to pixels
protected:
  double conv;

protected:
  int checkSum;

protected:
  DviImpl * pDviImpl = nullptr;

protected:
  DviFontInfo dviInfo;
};
