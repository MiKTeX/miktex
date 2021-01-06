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

#include <atomic>
#include <chrono>
#include <thread>

#include <miktex/Util/PathName>

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace MiKTeX::Util;

BEGIN_TEST_SCRIPT("fndb-2");

atomic_bool stopped;

bool FindFiles(size_t rounds)
{
  PathName path;
  for (size_t i = 0; i < rounds; ++i)
  {
    if (!(pSession->FindFile("test.tex", "%R/tex//", path) && pSession->FindFile("test.cls", "%R/tex/latex//", path)))
    {
      return false;
    }
  }
  return true;
}

BEGIN_TEST_FUNCTION(1);
{
  while (!stopped)
  {
    TEST(FindFiles(1001));
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}
END_TEST_FUNCTION();

BEGIN_TEST_FUNCTION(2);
{
  while (!stopped)
  {
    TEST(FindFiles(102));
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}
END_TEST_FUNCTION();

void Thread1()
{
  try
  {
    CALL_TEST_FUNCTION(1);
  }
  catch (const TestScript::FAILURE & f)
  {
    LOG4CXX_FATAL(logger, "Thread1 failed in " << f.strFile << ", line " << f.iLine);
  }
  catch (const MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
  }
}

void Thread2()
{
  try
  {
    size_t nSuccess = 0;
    size_t nFailure = 0;
    while (!stopped)
    {
      while (!pSession->UnloadFilenameDatabase())
      {
        ++nFailure;
        this_thread::sleep_for(chrono::milliseconds(10));
      }
      ++nSuccess;
      this_thread::sleep_for(chrono::milliseconds(10));
    }
    LOG4CXX_INFO(logger, "UnloadFilenameDatabase succ/fail: " << nSuccess << "/" << nFailure);
  }
  catch (const TestScript::FAILURE & f)
  {
    LOG4CXX_FATAL(logger, "Thread2 failed in " << f.strFile << ", line " << f.iLine);
  }
  catch (const MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
  }
}

void Thread3()
{
  try
  {
    CALL_TEST_FUNCTION(2);
  }
  catch (const FAILURE & f)
  {
    LOG4CXX_FATAL(logger, "Thread3 failed in " << f.strFile << ", line " << f.iLine);
  }
  catch (const MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
  }
}

BEGIN_TEST_PROGRAM();
{
  stopped = false;

  thread thread1(&MyTestScript::Thread1, this);

  this_thread::sleep_for(chrono::milliseconds(100));

  thread thread2(&MyTestScript::Thread2, this);

  this_thread::sleep_for(chrono::milliseconds(100));

  thread thread3(&MyTestScript::Thread3, this);

  this_thread::sleep_for(chrono::milliseconds(30000));

  stopped = true;

  thread1.join();
  thread2.join();
  thread3.join();
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
