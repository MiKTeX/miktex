/* TemporaryDirectory.cpp:

   Copyright (C) 2016-2018 Christian Schenk

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

#if defined(MIKTEX_UNIX)
#  include <sys/stat.h>
#endif

#include <random>

#include <miktex/Core/Directory>
#include <miktex/Core/TemporaryDirectory>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;

TemporaryDirectory::~TemporaryDirectory() noexcept
{
}

class TemporaryDirectoryImpl :
  public TemporaryDirectory
{
public:
  TemporaryDirectoryImpl()
  {
    shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
    PathName parent;
    if (session != nullptr)
    {
      parent = session->GetTempDirectory();
    }
    else
    {
      parent.SetToTempDirectory();
    }
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1,99999);
    for (size_t maxrounds = 10; maxrounds > 0; --maxrounds)
    {
      string fileName = "mik" + std::to_string(dist(gen));
      path = parent / fileName;
      if (!NameExists(path))
      {
        Directory::Create(path);
        return;
      }
    }
    MIKTEX_FATAL_ERROR_2(T_("Could not create a unique sub-directory."), "tempDir", parent.ToString());
  }

public:
  TemporaryDirectoryImpl(const PathName& path)
  {
    this->path = path;
  }

public:
  ~TemporaryDirectoryImpl() override
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
    Directory::Delete(path, true);
    path = "";
  }

public:
  PathName MIKTEXTHISCALL GetPathName() override
  {
    return path;
  }

public:
  void MIKTEXTHISCALL Keep() override
  {
    path = "";
  }

private:
  static bool NameExists(const PathName& name)
  {
#if defined(MIKTEX_WINDOWS)
    unsigned long attributes = GetFileAttributesW(name.ToWideCharString().c_str());
    return attributes != INVALID_FILE_ATTRIBUTES;
#else
    struct stat statbuf;
    return stat(name.GetData(), &statbuf) == 0;
#endif
  }

private:
  PathName path;
};

unique_ptr<TemporaryDirectory> TemporaryDirectory::Create()
{
  return make_unique<TemporaryDirectoryImpl>();
}

unique_ptr<TemporaryDirectory> TemporaryDirectory::Create(const PathName& path)
{
  return make_unique<TemporaryDirectoryImpl>(path);
}
