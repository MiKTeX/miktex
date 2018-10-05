/* vfchar.h:                                            -*- C++ -*-

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
class InputStream;

class VfChar : public DviChar
{
public:
  VfChar(DviFont * pFont = nullptr);

public:
  virtual ~VfChar();

public:
  void Read(InputStream & inputStream, int size, double conv);

public:
  const BYTE * GetPacket(unsigned long & length)
  {
    length = packetSize;
    return pPacket;
  }

private:
  int packetSize = 0;

private:
  BYTE smallPacket[1];

private:
  BYTE * pPacket = nullptr;

private:
  unique_ptr<TraceStream> trace_vfchar;
};
