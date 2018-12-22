/* 1.cpp:

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

#include "config.h"

#include <miktex/Core/Test>

#include <string>
#include <vector>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Core/Fndb>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;
using namespace std;

BEGIN_TEST_SCRIPT("fndb-1");

BEGIN_TEST_FUNCTION(1);
{
  PathName localRoot = pSession->GetSpecialPath(SpecialPath::DataRoot);

  PathName installRoot = pSession->GetSpecialPath(SpecialPath::InstallRoot);

  PathName pathConfigDir(localRoot);
  pathConfigDir /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  TESTX(Directory::Create(pathConfigDir));

  TEST(Directory::Exists(localRoot));

  unsigned localRootIdx = pSession->DeriveTEXMFRoot(localRoot);
  PathName fndbLocal = pSession->GetFilenameDatabasePathName(localRootIdx);
  TEST(Fndb::Create(fndbLocal.GetData(), localRoot.GetData(), nullptr));
  TEST(File::Exists(fndbLocal));

  unsigned installRootIdx = pSession->DeriveTEXMFRoot(installRoot);
  PathName fndbInstall = pSession->GetFilenameDatabasePathName(installRootIdx);
  TEST(Fndb::Create(fndbInstall.GetData(), installRoot.GetData(), nullptr));
  TEST(File::Exists(fndbInstall));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  PathName path;
  TEST(pSession->FindFile("test.tex", "%R/tex//", path));
  if (File::Exists("./test.tex"))
  {
    File::Delete("./test.tex");
  }
  TEST(!pSession->FindFile("./test.tex", "%R/tex//", path));
  Touch("./test.tex");
  TEST(pSession->FindFile("./test.tex", "%R/tex//", path));
  TEST(pSession->FindFile("./test.tex", StringUtil::Flatten({ ".", "%R/tex//" }, PathName::PathNameDelimiter), path));
  path.MakeAbsolute();
  PathName path2;
  path2.SetToCurrentDirectory();
  path2 /= "test.tex";
  TEST(path == path2);
  File::Delete("./test.tex");
  TEST(pSession->FindFile("test.tex", "%R/tex//base", path));
  TEST(pSession->FindFile("base/test.tex", "%R/tex//", path));
  vector<PathName> paths;
  TEST(pSession->FindFile("xyz.txt", StringUtil::Flatten({ "%R/ab//", "%R/jk//" }, PathName::PathNameDelimiter), paths));
  TEST(paths.size() == 2);
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  vector<PathName> paths;
  TEST(pSession->FindFile("xyz.txt", StringUtil::Flatten({ "%R/ab//", "%R/jk//" }, PathName::PathNameDelimiter), paths));
  TEST(!paths.empty());
  for (const PathName& p : paths)
  {
    TEST(Fndb::FileExists(p));
  }
  PathName path = pSession->GetSpecialPath(SpecialPath::InstallRoot) / "abrakadabra" / "hi.txt";
  TESTX(Fndb::Add({ {PathName(path)} }));
  TESTX(pSession->UnloadFilenameDatabase());
  TEST(Fndb::FileExists(path));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
  CALL_TEST_FUNCTION(3);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();

