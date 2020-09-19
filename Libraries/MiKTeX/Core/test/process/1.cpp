/* 1.cpp:

   Copyright (C) 1996-2017 Christian Schenk

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
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/TemporaryFile>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("process-1");

BEGIN_TEST_FUNCTION(1);
{
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_process_test1-1" MIKTEX_EXE_FILE_SUFFIX;
  int exitCode;
  TEST(Process::Run(pathExe, { pathExe.ToString(), "1-1.txt" }, nullptr, &exitCode, nullptr));
  TEST(exitCode == 0);
  TEST(File::Exists(PathName("1-1.txt")));
  TESTX(File::Delete(PathName("1-1.txt")));
}
END_TEST_FUNCTION();

string outputBuffer;

BEGIN_TEST_FUNCTION(2);
{
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_process_test1-2" MIKTEX_EXE_FILE_SUFFIX;
  int exitCode;
  ProcessOutput<1024> processOutput;
  TEST(Process::Run(pathExe, { pathExe.ToString(), "hello", "world!" }, &processOutput, &exitCode, nullptr));
  TEST(exitCode == 0);
  TEST(processOutput.StdoutToString() == "hello\nworld!\n");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(3);
{
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_process_test1-3" MIKTEX_EXE_FILE_SUFFIX;
  int exitCode;
  MiKTeXException ex;
  TEST(Process::Run(pathExe, { pathExe.ToString() }, nullptr, &exitCode, &ex, nullptr));
  TEST(exitCode == 1);
  TEST(ex.GetProgramInvocationName() == "process-1-3");
  TEST(ex.GetErrorMessage() == "xerrorMessagey");
  TEST(ex.GetDescription() == "xdescriptiony");
  TEST(ex.GetRemedy() == "xremedyy");
  TEST(ex.GetTag() == "tag");
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(4);
{
  unique_ptr<TemporaryFile> tmpFile = TemporaryFile::Create();
  MiKTeXException ex(
    "foo",
    "abrakadabra",
    "aaa{fileName}zzz",
    "xxx{exitCode}yyy",
    "a-b-c",
    MiKTeXException::KVMAP(
      "fileName", "foo.txt",
      "exitCode", "0"),
    SourceLocation());
  TEST(ex.Save(tmpFile->GetPathName().ToString()));
  MiKTeXException ex2;
  TEST(MiKTeXException::Load(tmpFile->GetPathName().ToString(), ex2));
  TEST(ex2.GetProgramInvocationName() == "foo");
  TEST(ex2.GetDescription() == "aaafoo.txtzzz");
  TEST(ex2.GetRemedy() == "xxx0yyy");
  TEST(ex2.GetTag() == "a-b-c");
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
