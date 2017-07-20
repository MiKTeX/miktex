/* 2.cpp:

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

#include <string>

#include <miktex/Core/File>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("process-2");

BEGIN_TEST_FUNCTION(1);
{
  std::string commandLine;
  commandLine = "echo 0123456789 > a.txt";
  int exitCode;
  TEST(MiKTeX::Core::Process::ExecuteSystemCommand(commandLine, &exitCode));
  TEST(exitCode == 0);
  TEST(MiKTeX::Core::File::Exists("a.txt"));
  TESTX(MiKTeX::Core::File::Delete("a.txt"));
}
END_TEST_FUNCTION();

std::string outputBuffer;

BEGIN_TEST_FUNCTION(2);
{
  std::string commandLine;
  commandLine = "echo 0123456789";
  int exitCode;
  MiKTeX::Core::ProcessOutput<1024> processOutput;
  TEST(MiKTeX::Core::Process::ExecuteSystemCommand(commandLine, &exitCode, &processOutput, nullptr));
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
