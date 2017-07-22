/* miktex/PipeStream.h:

   Copyright (C) 2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include "config.h"

#include <miktex/Core/Process>
#include <miktex/Core/Session>

#define MIKTEX_BEGIN_NS                         \
namespace MiKTeX {                              \
  namespace Aymptote {

#define MIKTEX_END_NS                           \
  }                                             \
}

MIKTEX_BEGIN_NS;

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
    if (process != nullptr)
    {
      process->WaitForExit(1000);
    }
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
public:
  void Write(const void* buf, size_t size)
  {
    if (fwrite(buf, 1, size, inFile) != size)
    {
      MIKTEX_FATAL_CRT_ERROR("fwrite");
    }
  }
public:
  size_t Read(void* buf, size_t size)
  {
    try
    {
      size_t n = fread(buf, 1, size, outFile);
      if (ferror(outFile) != 0)
      {
        MIKTEX_FATAL_CRT_ERROR("fread");
      }
      return n;
    }
    catch (const MiKTeX::Core::BrokenPipeException&)
    {
      return 0;
    }
  }
public:
  int Wait()
  {
    process->WaitForExit();
    return process->get_ExitCode();
  }
private:
  std::unique_ptr<MiKTeX::Core::Process> process;
};

MIKTEX_END_NS;
