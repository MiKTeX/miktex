/* FileNameDatabase.cpp: file name database (public)

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

#include "StdAfx.h"

#include "internal.h"

#include "Session/SessionImpl.h"
#include "FileNameDatabase.h"

using namespace MiKTeX::Core;
using namespace std;

void Fndb::Add(const char * lpszPath)
{
  Fndb::Add(lpszPath, nullptr);
}

void Fndb::Add(const char * lpszPath, const char * lpszFileNameInfo)
{
  MIKTEX_ASSERT_STRING(lpszPath);

#if 0
  MIKTEX_ASSERT(File::Exists(lpszPath));
#endif

  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(lpszPath);
  PathName pathFqFndbFileName;
  if (session->FindFilenameDatabase(root, pathFqFndbFileName))
  {
    shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root, TriState::False);
    if (fndb == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    fndb->AddFile(lpszPath, lpszFileNameInfo);
  }
  else
  {
    // create the fndb file
    // FIXME: the file name info hasn't been added, if the file exists
    PathName pathFndbFile = session->GetFilenameDatabasePathName(root);
    if (!Fndb::Create(pathFndbFile.GetData(), session->GetRootDirectory(root).GetData(), nullptr))
    {
      MIKTEX_UNEXPECTED();
    }
    if (!File::Exists(lpszPath))
    {
      // the file hasn't been added yet
      // RECURSION
      Add(lpszPath, lpszFileNameInfo);
    }
  }
}

bool Fndb::Enumerate(const char * lpszPath, IEnumerateFndbCallback * pCallback)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(lpszPath);
  if (fndb == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The path is not covered by the file name database."), "path", lpszPath);
  }
  return fndb->Enumerate(lpszPath, pCallback);
}

void Fndb::Remove(const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(lpszPath);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root, TriState::False);
  if (fndb == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  fndb->RemoveFile(lpszPath);
}

bool Fndb::FileExists(const PathName & path)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(path);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root, TriState::True);
  if (fndb == nullptr)
  {
    return false;
  }
  return fndb->FileExists(path);
}

bool Fndb::Search(const PathName & fileName, const char * pathPattern, bool firstMatchOnly, vector<PathName> & result, vector<string> & fileNameInfo)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned root = session->DeriveTEXMFRoot(pathPattern);
  shared_ptr<FileNameDatabase> fndb = session->GetFileNameDatabase(root, TriState::True);
  if (fndb == nullptr)
  {
    return false;
  }
  return fndb->Search(fileName, pathPattern, firstMatchOnly, result, fileNameInfo);
}
