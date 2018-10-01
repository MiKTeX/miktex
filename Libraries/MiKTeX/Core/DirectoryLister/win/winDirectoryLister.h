/* winDirectoryLister.h: directory lister		-*- C++ -*-

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

#pragma once

#if !defined(F05A2700545C4487889CF7A396F930A1)
#define F05A2700545C4487889CF7A396F930A1

#include "miktex/Core/DirectoryLister.h"
#include "miktex/Core/PathName.h"

BEGIN_INTERNAL_NAMESPACE;

class winDirectoryLister : public MiKTeX::Core::DirectoryLister
{
public:
  virtual void MIKTEXTHISCALL Close();

public:
  virtual bool MIKTEXTHISCALL GetNext(MiKTeX::Core::DirectoryEntry & direntry);

public:
  virtual bool MIKTEXTHISCALL GetNext(MiKTeX::Core::DirectoryEntry2 & direntry2);

public:
  winDirectoryLister(const MiKTeX::Core::PathName & directory, const char * lpszPattern, int options);

public:
  virtual MIKTEXTHISCALL ~winDirectoryLister();

private:
  MiKTeX::Core::PathName directory;

private:
  std::string pattern;

private:
  int options = (int)Options::None;

private:
  HANDLE handle = INVALID_HANDLE_VALUE;

private:
  friend class MiKTeX::Core::DirectoryLister;
};

END_INTERNAL_NAMESPACE;

#endif
