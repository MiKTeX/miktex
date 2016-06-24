/* Ghostscript.h:                                       -*- C++ -*-

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

#include "PostScript.h"

class Ghostscript :
  public PostScript,
  public IDibChunkerCallback
{
public:
  virtual ~Ghostscript();

private:
  virtual void Finalize();

public:
  virtual void Write(const void * p, unsigned n);

public:
  virtual void Execute(const char * lpszFormat, ...);

public:
  virtual size_t MIKTEXTHISCALL Read(void * pBuf, size_t size);

public:
  virtual void MIKTEXTHISCALL OnNewChunk(shared_ptr<DibChunk> pChunk);

public:
  Ghostscript();

private:
  void Start();

private:
  void Chunker();

private:
  void StderrReader();

private:
  thread chunkerThread;

private:
  thread stderrReaderThread;

private:
  FileStream gsIn;

private:
  FileStream gsOut;

private:
  FileStream gsErr;

private:
  unique_ptr<Process> pProcess;

private:
  string stderrBuffer;
};
