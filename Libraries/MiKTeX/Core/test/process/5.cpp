/* 5.cpp:

   Copyright (C) 2018 Christian Schenk

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

#include <chrono>
#include <thread>

#include <miktex/Core/Test>

#include <miktex/Core/Paths>
#include <miktex/Core/Process>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

BEGIN_TEST_SCRIPT("process-5");

BEGIN_TEST_FUNCTION(1);
{
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_process_test5-1" MIKTEX_EXE_FILE_SUFFIX;
  int exitCode;
  TEST(!File::Exists("5-1.txt"));
  TEST(Process::Run(pathExe, { pathExe.ToString(), "5-1.txt" }, nullptr, &exitCode, nullptr));
  TEST(exitCode == 0);
  std::this_thread::sleep_for(11s);
  TEST(File::Exists("5-1.txt"));
  TESTX(File::Delete("5-1.txt"));
}
END_TEST_FUNCTION();

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
