/* DviChar.h:                                           -*- C++ -*-

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

class DviFont;

class DviChar
{

public:
  DviChar(DviFont * pDviFont = nullptr);

public:
  virtual ~DviChar();

public:
  int GetWidth()
  {
    return cx;
  }

public:
  void SetWidth(int cx)
  {
    this->cx = cx;
  }

public:
  int GetDviWidth()
  {
    return tfm;
  }

public:
  void SetDviWidth(int tfm)
  {
    this->tfm = tfm;
  }

public:
  int GetCharacterCode()
  {
    return charCode;
  }

public:
  void SetCharacterCode(int charCode)
  {
    this->charCode = charCode;
  }

public:
  inline int GetScaledAt();

  // character code
protected:
  int charCode = 0;

  // glyph width (in DVI units)
protected:
  int tfm = 0;

  // horizontal escapement (in pixels)
protected:
  int cx = 0;

  // vertical escapement (in pixels)
protected:
  int cy = 0;

protected:
  DviFont * pDviFont = nullptr;
};
