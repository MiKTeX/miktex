/* miktex/Core/Fndb.h:                                  -*- C++ -*-

   Copyright (C) 1996-2019 Christian Schenk

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

#pragma once

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
  virtual bool MIKTEXTHISCALL ReadDirectory(const PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) = 0;

public:
  virtual bool MIKTEXTHISCALL OnProgress(unsigned level, const PathName& directory) = 0;
};

class MIKTEXNOVTABLE Fndb
{
public:
  Fndb() = delete;

public:
  Fndb(const Fndb& other) = delete;

public:
  Fndb& operator=(const Fndb& other) = delete;

public:
  Fndb(Fndb&& other) = delete;

public:
  Fndb& operator= (Fndb&& other) = delete;

public:
  ~Fndb() = delete;

public:
  struct Record
  {
    PathName path;
    std::string fileNameInfo;
  };

public:
  static MIKTEXCORECEEAPI(bool) Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback);

public:
  static MIKTEXCORECEEAPI(bool) Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback, bool enableStringPooling, bool storeFileNameInfo);

public:
  static MIKTEXCORECEEAPI(bool) Search(const PathName& fileName, const std::string& pathPattern, bool all, std::vector<Record>& result);

public:
  static MIKTEXCORECEEAPI(void) Add(const std::vector<Record>& records);

public:
  static MIKTEXCORECEEAPI(void) Remove(const std::vector<PathName>& paths);

public:
  static MIKTEXCORECEEAPI(bool) FileExists(const PathName& path);

public:
  static MIKTEXCORECEEAPI(bool) Refresh(const PathName& path, ICreateFndbCallback* callback);

public:
  static MIKTEXCORECEEAPI(bool) Refresh(ICreateFndbCallback* callback);

};

MIKTEX_CORE_END_NAMESPACE;

#endif
