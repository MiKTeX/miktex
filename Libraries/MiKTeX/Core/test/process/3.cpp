/* 3.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include <cstdio>

#include <memory>
#include <string>
#include <vector>

#include <miktex/Core/Test>

#include <miktex/Core/File>
#include <miktex/Core/PathName>
#include <miktex/Core/Process>

BEGIN_TEST_SCRIPT();

BEGIN_TEST_FUNCTION(1);
{
  TEST (Process::ExecuteSystemCommand("echo 0123456789x> a.txt"));
  FILE * pFileIn = File::Open("a.txt", FileMode::Open, FileAccess::Read, false);
  TEST (pFileIn != 0);
  PathName pathExe = pSession->GetSpecialPath(SpecialPath::BinDirectory);
  pathExe += "3-1" MIKTEX_EXE_FILE_SUFFIX;
  FILE * pFileChildOut;
  TESTX (Process::Start(pathExe.Get(), 0, pFileIn, 0, &pFileChildOut, 0, 0));
  size_t n;
  std::string str;
  char buf[100];
  while ((n = fread(buf, 1, 100, pFileChildOut)) > 0)
    {
      str.append (buf, n);
    }
  fclose (pFileIn);
  fclose (pFileChildOut);
#if defined(MIKTEX_WINDOWS)
  TEST (str == "0123456789x\r\n");
#else
  TEST (str == "0123456789x\n");
#endif
  TESTX (File::Delete("a.txt"));
}
END_TEST_FUNCTION();

#if defined(MIKTEX_WINDOWS)
BEGIN_TEST_FUNCTION(2);
{
  TEST (Process::ExecuteSystemCommand("echo 0123456789x> a.txt"));
  FILE * pFileIn = File::Open("a.txt", FileMode::Open, FileAccess::Read, false);
  TEST (pFileIn != nullptr);
  PathName pathExe = pSession->GetSpecialPath(SpecialPath::BinDirectory);
  pathExe += "3-1" MIKTEX_EXE_FILE_SUFFIX;
  FILE * pFileChildOut = File::Open("b.txt", FileMode::Create, FileAccess::Write, false);
  TEST (pFileChildOut != nullptr);
  ProcessStartInfo psi (pathExe.Get());
  psi.StandardInput = pFileIn;
  psi.StandardOutput = pFileChildOut;
  unique_ptr<Process> pProcess (Process::Start(psi));
  TEST (pProcess->WaitForExit(2000));
  fclose (pFileIn);
  fclose (pFileChildOut);
  std::vector<unsigned char> vec1 (File::ReadAllBytes("a.txt"));
  std::vector<unsigned char> vec2 (File::ReadAllBytes("b.txt"));
  TEST (vec1 == vec2);
  TESTX (File::Delete("a.txt"));
  TESTX (File::Delete("b.txt"));
}
END_TEST_FUNCTION();
#endif

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION (1);
#if defined(MIKTEX_WINDOWS)
  CALL_TEST_FUNCTION (2);
#endif
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT ();
