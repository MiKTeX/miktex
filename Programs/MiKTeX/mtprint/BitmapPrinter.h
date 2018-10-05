/* BitmapPrinter.h:                                     -*- C++ -*-

   Copyright (C) 2003-2016 Christian Schenk

   This file is part of MTPrint.

   MTPrint is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   MTPrint is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MTPrint; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#pragma once

#include "Printer.h"

class BitmapPrinter :
  public Printer,
  public IDibChunkerCallback
{
public:
  BitmapPrinter(const Printer::PRINTINFO & printinfo, bool pintNothing = false);

public:
  virtual size_t MIKTEXTHISCALL Read(void * pBuf, size_t size);

public:
  virtual void MIKTEXTHISCALL OnNewChunk(shared_ptr<DibChunk> pChunk);

public:
  void Print(FILE * pfileDibStream);

private:
  void PrintChunk(const DibChunk & chunk);

private:
  int offsetX;

private:
  int offsetY;

private:
  FileStream stream;
};
