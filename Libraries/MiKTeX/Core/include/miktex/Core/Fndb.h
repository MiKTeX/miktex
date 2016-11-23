/* miktex/Core/Fndb.h:                                  -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(BFCDDE5CC44547308426241D426A676E)
#define BFCDDE5CC44547308426241D426A676E

#include <miktex/Core/config.h>

#include <string>
#include <vector>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE ICreateFndbCallback
{
public:
  virtual bool MIKTEXTHISCALL ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos) = 0;

public:
  virtual bool MIKTEXTHISCALL OnProgress(unsigned level, const char * lpszDirectory) = 0;
};

class MIKTEXNOVTABLE IEnumerateFndbCallback
{
public:
  virtual bool MIKTEXTHISCALL OnFndbItem(const char * lpszPath, const char * lpszName, const char * lpszInfo, bool isDirectory) = 0;
};

class MIKTEXNOVTABLE Fndb
{
public:
  static MIKTEXCORECEEAPI(void) Add(const char * lpszPath);

public:
  static void Add(const PathName & path)
  {
    Add(path.GetData());
  }

public:
  static MIKTEXCORECEEAPI(void) Add(const char * lpszPath, const char * lpszFileNameInfo);

public:
  static void Add(const PathName & path, const char * lpszFileNameInfo)
  {
    Add(path.GetData(), lpszFileNameInfo);
  }

public:
  static MIKTEXCORECEEAPI(bool) Create(const char * lpszFndbPath, const char * lpszRootPath, ICreateFndbCallback * callback);

public:
  static bool Create(const PathName & fndbPath, const PathName & rootPath, ICreateFndbCallback * callback)
  {
    return Create(fndbPath.GetData(), rootPath.GetData(), callback);
  }

public:
  static MIKTEXCORECEEAPI(bool) Create(const char * lpszFndbPath, const char * lpszRootPath, ICreateFndbCallback * callback, bool enableStringPooling, bool storeFileNameInfo);

public:
  static MIKTEXCORECEEAPI(bool) Enumerate(const char * lpszPath, IEnumerateFndbCallback * callback);

public:
  static bool Enumerate(const PathName & path, IEnumerateFndbCallback * callback)
  {
    return Enumerate(path.GetData(), callback);
  }

public:
  static MIKTEXCORECEEAPI(bool) Refresh(const PathName & path, ICreateFndbCallback * callback);

public:
  static MIKTEXCORECEEAPI(bool) Refresh(ICreateFndbCallback * callback);

public:
  static MIKTEXCORECEEAPI(void) Remove(const char * lpszPath);

public:
  static void Remove(const PathName & path)
  {
    Remove(path.GetData());
  }

public:
  static MIKTEXCORECEEAPI(bool) FileExists(const PathName & path);

public:
  static MIKTEXCORECEEAPI(bool) Search(const char * lpszFileName, const char * lpszPathPattern, bool firstMatchOnly, std::vector<PathName> & result, std::vector<std::string> & fileNameInfo);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
