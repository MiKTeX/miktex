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

#include <memory>
#include <mutex>
#include <thread>

#include <miktex/Core/Exceptions>

using namespace MiKTeX::Core;
using namespace MiKTeX::Test;
using namespace std;

#if 0
#define N 1000000
#else
#define N 10000
#endif

#define RECURSIVE 0

BEGIN_TEST_SCRIPT("thread-1");

mutex startedMutex;
mutex stoppedMutex;
mutex nMutex;

volatile bool started;
volatile bool stopped;

volatile int n;
volatile int n1;
volatile int n2;

BEGIN_TEST_FUNCTION(1);
{
  TEST(n == N);
  TEST(n1 + n2 == n);
}
END_TEST_FUNCTION();

void Incrementer1()
{
  for (;;)
  {
    {
      lock_guard<mutex> lockGuard(startedMutex);
      if (started)
      {
	break;
      }
    }
    this_thread::yield();
  }
  for (;;)
  {
    {
      lock_guard<mutex> lockGuard(stoppedMutex);
      if (stopped)
      {
	break;
      }
    }
    unsigned wait = 0;
    {
      lock_guard<mutex> lockGuard(nMutex);
#if RECURSIVE
      {
        lock_guard<mutex> lockGuard(nMutex);
	{
          lock_guard<mutex> lockGuard(nMutex);
#endif
	  if (n == N)
	  {
	    return;
	  }
	  ++n;
	  ++n1;
	  if (n % 7 == 0)
	  {
	    wait = 1;
	  }
#if RECURSIVE
	}
      }
#endif
    }
    this_thread::sleep_for(chrono::milliseconds(wait));
  }
}

void Incrementer2()
{
  for (;;)
  {
    {
      lock_guard<mutex> lockGuard(startedMutex);
      if (started)
      {
	break;
      }
    }
    this_thread::yield();
  }
  for (;;)
  {
    {
      lock_guard<mutex> lockGuard(stoppedMutex);
      if (stopped)
      {
	break;
      }
    }
    unsigned wait = 0;
    {
      lock_guard<mutex> lockGuard(nMutex);
#if RECURSIVE
      {
        lock_guard<mutex> lockGuard(nMutex);
	{
          lock_guard<mutex> lockGuard(nMutex);
#endif
	  if (n == N)
	  {
	    return;
	  }
	  ++n;
	  ++n2;
	  if (n % 7 == 0)
	  {
	    wait = 1;
	  }
#if RECURSIVE
	}
      }
#endif
    }
    this_thread::sleep_for(chrono::milliseconds(wait));
  }
}

void Thread1()
{
  try
  {
    Incrementer1();
  }
  catch (const TestScript::FAILURE & f)
  {
    LOG4CXX_FATAL(logger, "Thread1 failed in " << f.strFile << ", line " << f.iLine);
  }
  catch (const MiKTeX::Core::MiKTeXException & e)
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
    Incrementer2();
  }
  catch (const TestScript::FAILURE & f)
  {
    LOG4CXX_FATAL(logger, "Thread2 failed in " << f.strFile << ", line " << f.iLine);
  }
  catch (const MiKTeX::Core::MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
  }
}

BEGIN_TEST_PROGRAM();
{
  started = false;
  stopped = false;
  n = 0;
  n1 = 0;
  n2 = 0;

  thread thread1(&MyTestScript::Thread1, this);
  thread thread2(&MyTestScript::Thread2, this);

  {
    lock_guard<mutex> lockGuard(startedMutex);
    started = true;
  }

  thread1.join();
  thread2.join();

  LOG4CXX_INFO(logger, n1 << "+" << n2);

  CALL_TEST_FUNCTION(1);
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();

