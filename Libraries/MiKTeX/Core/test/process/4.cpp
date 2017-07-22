/* 4.cpp:

   Copyright (C) 2017 Christian Schenk

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

class PipeStream
{
public:
  virtual ~PipeStream()
  {
    Close();
  }
public:
  void Open(const MiKTeX::Core::PathName& fileName, const std::vector<std::string>& arguments)
  {
    MiKTeX::Core::ProcessStartInfo startInfo;
    startInfo.FileName = fileName.ToString();
    startInfo.Arguments = arguments;
    startInfo.RedirectStandardInput = true;
    startInfo.RedirectStandardError = true;
    startInfo.RedirectStandardOutput = true;
    process = MiKTeX::Core::Process::Start(startInfo);
    inFile = process->get_StandardInput();
    outFile = process->get_StandardOutput();
    errFile = process->get_StandardError();
  }
public:
  void Close()
  {
    CloseIn();
    CloseOut();
    CloseErr();
    process->WaitForExit(1000);
  }
public:
  PipeStream& operator<<(const std::string& s)
  {
    Write(s);
    return *this;
  }
  PipeStream& operator>>(std::string& s)
  {
    s = Read();
    return *this;
  }
public:
  void CloseIn()
  {
    if (inFile != nullptr)
    {
      fclose(inFile);
      inFile = nullptr;
    }
  }
protected:
  void CloseOut()
  {
    if (outFile != nullptr)
    {
      fclose(outFile);
      outFile = nullptr;
    }
  }
protected:
  void CloseErr()
  {
    if (errFile != nullptr)
    {
      fclose(errFile);
      errFile = nullptr;
    }
  }
private:
  FILE* inFile = nullptr;
private:
  FILE* outFile = nullptr;
private:
  FILE* errFile = nullptr;
private:
  void Write(const std::string& s)
  {
    if (fwrite(s.c_str(), 1, s.length(), inFile) != s.length())
    {
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }
private:
  std::string Read()
  {
    std::string s;
    char buf[1024];
    size_t n = fread(buf, 1, 1024, outFile);
    if (ferror(outFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fread");
    }
    s.assign(buf, n);
    return s;
  }
private:
  std::unique_ptr<MiKTeX::Core::Process> process;
};

BEGIN_TEST_SCRIPT("process-4");

BEGIN_TEST_FUNCTION(1);
{
#if 1
  PipeStream pipe;
  PathName pathExe = pSession->GetMyLocation(false);
  pathExe /= "core_process_test3-1" MIKTEX_EXE_FILE_SUFFIX;
  pipe.Open(pathExe, { pathExe.ToString() });
  pipe << "Hello, world!";
  pipe.CloseIn();
  string s;
  pipe >> s;
  TEST(s == "Hello, world!");
#endif
}
END_TEST_FUNCTION();

#if defined(MIKTEX_WINDOWS)
BEGIN_TEST_FUNCTION(2);
{
}
END_TEST_FUNCTION();
#endif

BEGIN_TEST_PROGRAM();
{
  CALL_TEST_FUNCTION(1);
#if defined(MIKTEX_WINDOWS)
  CALL_TEST_FUNCTION(2);
#endif
}
END_TEST_PROGRAM();

END_TEST_SCRIPT();

RUN_TEST_SCRIPT();
