/* PostScript.h:                                        -*- C++ -*-

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

class PostScript
{
protected:
  virtual ~PostScript();

protected:
  virtual void Finalize();

protected:
  virtual void Write(const void * p, unsigned n) = 0;

protected:
  virtual void Execute(const char * lpszFormat, ...) = 0;

protected:
  PostScript();

public:
  shared_ptr<GraphicsInclusion> GetGraphicsInclusion(int idx)
  {
    if (idx == graphicsInclusions.size())
    {
      return nullptr;
    }
    else if (idx > graphicsInclusions.size())
    {
      MIKTEX_UNEXPECTED();
    }
    return graphicsInclusions[idx];
  }

public:
  bool IsOpen() const
  {
    return openFlag;
  }

public:
  bool IsPageEmpty() const
  {
    return !pageBegunFlag;
  }

public:
  void AddDefinition(class PsdefSpecial * ppsdefspecial);

public:
  void BeginPage();

public:
  void Close();

public:
  void DoSpecial(class PsfileSpecial * ppsfilespecial);

public:
  void DoSpecial(class DvipsSpecial * pdvipsspecial);

public:
  void EndPage();

public:
  void Open(class DviImpl * pDviImpl, int shrinkFactor);

protected:
  void AddHeader(const char * lpszFileName);

protected:
  FILE * ConvertToEPS(const char * lpszFileName);

private:
  void ConvertToEPSThread(PathName pathFile, FILE * pfileIn, FILE * pFileOut);

protected:
  void CopyFile(FileStream & stream, unsigned length);

protected:
  void DoDefinitions();

protected:
  void DoProlog();

protected:
  void ExecuteBatch(const char * lpszFileName);

protected:
  void ExecuteEncapsulatedPostScript(const char * lpszFileName);

protected:
  bool
    FindGraphicsFile(const char * lpszFileName, PathName & result);

protected:
  void Initialize();

protected:
  void SendHeader(const char * lpszHeaderName);

protected:
  void Uncompress(const char * lpszFileName, PathName & result);

protected:
  DviImpl * pDviImpl = nullptr;

protected:
  int shrinkFactor;

protected:
  bool openFlag = false;

protected:
  bool pageBegunFlag = false;

protected:
  vector<string> definitions;

protected:
  vector<string> headers;

protected:
  vector<shared_ptr<GraphicsInclusion> > graphicsInclusions;

protected:
  unique_ptr<TraceStream> tracePS;
};
