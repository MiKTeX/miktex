/* 1.cpp:

   Copyright (C) 2018-2020 Christian Schenk

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

#include <chrono>
#include <memory>
#include <thread>

#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/LockFile>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>

using namespace std;
using namespace std::chrono_literals;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;

BEGIN_TEST_SCRIPT("lockfile-1");

BEGIN_TEST_FUNCTION(1);
{
  unique_ptr<MiKTeX::Core::LockFile> lockFile = LockFile::Create(PathName("lockfile-1"));
  TEST(lockFile->TryLock(0s));
  {
    unique_ptr<MiKTeX::Core::LockFile> lockFile2 = LockFile::Create(PathName("lockfile-1"));
    lockFile->Unlock();
    TEST(lockFile2->TryLock(0s));
    TEST(File::Exists(PathName("lockfile-1")));
  }
  TEST(!File::Exists(PathName("lockfile-1")));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  unique_ptr<MiKTeX::Core::LockFile> lockFile = LockFile::Create(PathName("lockfile-1-1"));
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_lockfile_test1-1" MIKTEX_EXE_FILE_SUFFIX;
  TESTX(Process::Start(pathExe));
  this_thread::sleep_for(2s);
  TEST(!lockFile->TryLock(0s));
  TEST(lockFile->TryLock(10s));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  unique_ptr<MiKTeX::Core::LockFile> lockFile = LockFile::Create(PathName("lockfile-1-2"));
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_lockfile_test1-2" MIKTEX_EXE_FILE_SUFFIX;
  TESTX(Process::Start(pathExe));
  this_thread::sleep_for(2s);
  TEST(!lockFile->TryLock(0s));
  TEST(lockFile->TryLock(30s));
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_lockfile_test1-3" MIKTEX_EXE_FILE_SUFFIX;
  TESTX(Process::Start(pathExe));
  this_thread::sleep_for(2s);
  TEST(File::Exists(PathName("sharedfile-1")));
  FileStream reader(File::Open(PathName("sharedfile-1"), FileMode::Open, FileAccess::Read));
  TEST(!File::TryLock(reader.GetFile(), File::LockType::Shared, 0ms));
  TEST(File::TryLock(reader.GetFile(), File::LockType::Shared, 30s));
  reader.Close();
  TESTX(File::Delete(PathName("sharedfile-1")));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
  CALL_TEST_FUNCTION(2);
  CALL_TEST_FUNCTION(3);
  CALL_TEST_FUNCTION(4);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
