/* RootDirectoryInternals.h:                            -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#if !defined(FFCF19389D64414EBC1FB50A73140032)
#define FFCF19389D64414EBC1FB50A73140032

#include "miktex/Core/RootDirectoryInfo.h"
#include "Fndb/FileNameDatabase.h"

BEGIN_INTERNAL_NAMESPACE;

struct RootDirectoryInternals :
  MiKTeX::Core::RootDirectoryInfo
{
public:
  RootDirectoryInternals()
  {
  }

public:
  RootDirectoryInternals(const MiKTeX::Core::PathName& unexpandedPath, const MiKTeX::Core::PathName& path) :
    unexpandedPath(unexpandedPath)
  {
    this->path = path;
  }

public:
  const MiKTeX::Core::PathName& get_UnexpandedPath() const
  {
    return unexpandedPath;
  }

public:
  const MiKTeX::Core::PathName& get_Path() const
  {
    return path;
  }

public:
  void set_NoFndb(bool noFndb)
  {
    this->noFndb = noFndb;
  }

public:
  bool get_NoFndb() const
  {
    return noFndb;
  }

public:
  void set_Common(bool common)
  {
    if (common)
    {
      this->attributes += Attribute::Common;
    }
    else
    {
      this->attributes -= Attribute::Common;
    }
  }

public:
  void set_Other(bool other)
  {
    if (other)
    {
      this->attributes += Attribute::Other;
    }
    else
    {
      this->attributes -= Attribute::Other;
    }
  }

public:
  void SetFndb(std::shared_ptr<FileNameDatabase> fndb)
  {
    this->fndb = fndb;
  }

public:
  std::shared_ptr<FileNameDatabase> GetFndb() const
  {
    return fndb;
  }

private:
  MiKTeX::Core::PathName unexpandedPath;

  // associated file name database object
private:
  std::shared_ptr<FileNameDatabase> fndb;

  // true, if a file name database doesn't exist
private:
  bool noFndb = false;
};

END_INTERNAL_NAMESPACE;

#endif
