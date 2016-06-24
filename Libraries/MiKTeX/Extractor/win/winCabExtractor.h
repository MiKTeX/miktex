/* winCabExtractor.h:						-*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.
   
   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if defined(ENABLE_WINDOWS_CAB_EXTRACTOR)

#include "miktex/Extractor/Extractor"

BEGIN_INTERNAL_NAMESPACE;

class winCabExtractor : public Extractor
{
public:
  winCabExtractor ();

public:
  virtual
  MIKTEXTHISCALL
  ~winCabExtractor ();

public:
  virtual
  void
  MIKTEXTHISCALL
  Extract (/*[in]*/ const MiKTeX::Core::PathName &	cabinetPath,
	   /*[in]*/ const MiKTeX::Core::PathName &	destDir,
	   /*[in]*/ bool				makeDirectories,
	   /*[in]*/ IExtractCallback *			pCallback,
	   /*[in]*/ const char *			lpszPrefix);
  
public:
  virtual
  void
  MIKTEXTHISCALL
  Extract (/*[in]*/ MiKTeX::Core::Stream *		pStream,
	   /*[in]*/ const MiKTeX::Core::PathName &	destDir,
	   /*[in]*/ bool				makeDirectories,
	   /*[in]*/ IExtractCallback *			pCallback,
	   /*[in]*/ const char *			lpszPrefix);
  
public:
  virtual
  void
  MIKTEXTHISCALL
  Dispose ();

private:
  void
  MIKTEXNORETURN
  FatalFdiError (/*[in]*/ const char *	lpszCabinetPath);

private:
  IExtractCallback *	pCallback;

private:
  bool error;

private:
  bool cancelled;

private:
  bool makeDirectories;

private:
  unsigned long uncompressedSize;

private:
  static FNALLOC(Alloc);
  static FNFREE(Free);
  static FNOPEN(Open);
  static FNREAD(Read);
  static FNWRITE(Write);
  static FNCLOSE(Close);
  static FNSEEK(Seek);
  static FNFDINOTIFY(Notify);

private:
  HFDI hfdi;

private:
  ERF erf;

private:
  MiKTeX::Core::PathName destinationDirectory;

private:
  std::string prefixToBeStripped;

private:
  size_t fileCount;

private:
  std::auto_ptr<MiKTeX::Core::TraceStream> trace_error;

private:
  std::auto_ptr<MiKTeX::Core::TraceStream> trace_extractor;

private:
  static std::map<int, MiKTeX::Core::PathName> openFiles;
};

END_INTERNAL_NAMESPACE;

#endif
