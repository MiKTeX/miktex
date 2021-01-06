/* TemporaryFile.cpp:

   Copyright (C) 2016-2021 Christian Schenk

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

#include "config.h"

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/TemporaryFile>

#include <miktex/Util/PathName>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

TemporaryFile::~TemporaryFile() noexcept
{
}

class TemporaryFileImpl :
  public TemporaryFile
{
public:
  TemporaryFileImpl()
  {
    path.SetToTempFile();
  }

public:
  TemporaryFileImpl(const PathName& path)
  {
    if (Directory::Exists(path))
    {
      this->path.SetToTempFile(path);
    }
    else
    {
      this->path = path;
    }
  }

public:
  ~TemporaryFileImpl() override
  {
    try
    {
      if (!path.Empty())
      {
	Delete();
      }
    }
    catch (const exception&)
    {
    }
  }
  
public:
  void MIKTEXTHISCALL Delete() override
  {
    File::Delete(path);
    path = "";
  }

public:
  PathName MIKTEXTHISCALL GetPathName() const override
  {
    return path;
  }

public:
  void MIKTEXTHISCALL Keep() override
  {
    path = "";
  }

private:
  PathName path;
};

unique_ptr<TemporaryFile> TemporaryFile::Create()
{
  return make_unique<TemporaryFileImpl>();
}

unique_ptr<TemporaryFile> TemporaryFile::Create(const PathName& path)
{
  return make_unique<TemporaryFileImpl>(path);
}
