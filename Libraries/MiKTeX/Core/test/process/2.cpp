/* 2.cpp:

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

#include <string>

#include <miktex/Core/File>
#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("process-2");

BEGIN_TEST_FUNCTION(1);
{
  string commandLine = "echo 0123456789 > a.txt";
  int exitCode;
  TEST(Process::ExecuteSystemCommand(commandLine, &exitCode));
  TEST(exitCode == 0);
  TEST(File::Exists(PathName("a.txt")));
  TESTX(File::Delete(PathName("a.txt")));
}
END_TEST_FUNCTION();

std::string outputBuffer;

BEGIN_TEST_FUNCTION(2);
{
  string commandLine = "echo 0123456789";
  int exitCode;
  ProcessOutput<1024> processOutput;
  TEST(Process::ExecuteSystemCommand(commandLine, &exitCode, &processOutput, nullptr));
  TEST(exitCode == 0);
#if defined(MIKTEX_WINDOWS)
  TEST(processOutput.StdoutToString() == "0123456789\r\n");
#else
  TEST(processOutput.StdoutToString() == "0123456789\n");
#endif
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
