/* FileNameDatabase.cpp: file name database (public)

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "internal.h"

#include "Session/SessionImpl.h"
#include "FileNameDatabase.h"

using namespace std;

using namespace MiKTeX::Core;

bool Fndb::Search(const PathName& fileName, const string& pathPattern, bool firstMatchOnly, vector<Fndb::Record>& result)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(pathPattern);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root);
  if (fndb == nullptr)
  {
    return false;
  }
  return fndb->Search(fileName, pathPattern, firstMatchOnly, result);
}

void Fndb::Add(const vector<Fndb::Record>& records)
{
  if (records.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  // TODO: parse/split records
  unsigned root = session->DeriveTEXMFRoot(records[0].path);
  PathName pathFqFndbFileName;
  if (session->FindFilenameDatabase(root, pathFqFndbFileName))
  {
    shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root);
    if (fndb == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    fndb->Add(records);
  }
  else
  {
    // create the fndb file
    // FIXME: the file name info hasn't been added, if the file exists
    PathName pathFndbFile = session->GetFilenameDatabasePathName(root);
    if (!Fndb::Create(pathFndbFile.GetData(), session->GetRootDirectoryPath(root).GetData(), nullptr))
    {
      MIKTEX_UNEXPECTED();
    }
    // RECURSION
    Add(records);
  }
}

void Fndb::Remove(const vector<PathName>& paths)
{
  if (paths.empty())
  {
    MIKTEX_UNEXPECTED();
  }
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  // TODO: parse/split records
  unsigned root = session->DeriveTEXMFRoot(paths[0]);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root);
  if (fndb == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  fndb->Remove(paths);
}

bool Fndb::FileExists(const PathName& path)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(path);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root);
  if (fndb == nullptr)
  {
    return false;
  }
  return fndb->FileExists(path);
}
