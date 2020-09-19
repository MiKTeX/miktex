/* 1.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include <miktex/Core/Test>

#include <memory>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/PathName>
#include <miktex/Core/TemporaryDirectory>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("tempdir-1");

unique_ptr<TemporaryDirectory> Create(int n)
{
  if (n > 0)
  {
    unique_ptr<TemporaryDirectory> tmpDir = TemporaryDirectory::Create();
    for (int i = 1; i <= n; ++i)
    {
      Touch(tmpDir->GetPathName() / PathName(string("abrakadabra") + std::to_string(i)));
    }
    return tmpDir;
  }
  else
  {
    return nullptr;
  }
}

BEGIN_TEST_FUNCTION(1);
{
  PathName tmpDirPath;
  {
    unique_ptr<TemporaryDirectory> tmpdir;
    TESTX(tmpdir = TemporaryDirectory::Create());
    tmpDirPath = tmpdir->GetPathName();
    TEST(Directory::Exists(tmpDirPath));
    Touch(tmpDirPath / PathName("abrakadabra"));
    TEST(File::Exists(tmpDirPath / PathName("abrakadabra")));
  }
  TEST(!Directory::Exists(tmpDirPath));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  size_t n = 200;
  unique_ptr<TemporaryDirectory> tmpDir = Create(n);
  TEST(Directory::Exists(tmpDir->GetPathName()));
  for (int i = 1; i <= 200; ++i)
  {
    TEST(File::Exists(tmpDir->GetPathName() / PathName(string("abrakadabra") + std::to_string(i))));
  }
  PathName tmpDirPath = tmpDir->GetPathName();
  tmpDir->Delete();
  TEST(!Directory::Exists(tmpDirPath));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
