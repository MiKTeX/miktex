/* 1.cpp:

   Copyright (C) 1996-2021 Christian Schenk

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

#include <miktex/Core/File>
#include <miktex/Core/FileStream>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("file-1");

BEGIN_TEST_FUNCTION(1);
{
#if defined(MIKTEX_WINDOWS)
  Touch("xxx.zzz");
  vector<unique_ptr<FileStream>> files;
  size_t maxRounds = File::SetMaxOpen(2048);
  TEST(maxRounds > 100);
  maxRounds -= 100;
  TESTX(
    for (int i = 0; i < maxRounds; ++i)
    {
      files.push_back(make_unique<FileStream>(File::Open(PathName("xxx.zzz"), FileMode::Open, FileAccess::Read, false)));
    }
  );
  files.clear();
  File::Delete(PathName("xxx.zzz"));
#endif
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  FileStream file(File::Open(PathName("delete.me"), FileMode::Create, FileAccess::ReadWrite, false, { FileOpenOption::DeleteOnClose }));
  file.Close();
  TEST(!File::Exists(PathName("delete.me")));
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
