/* process.cpp: executing secondary processes

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Process.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

Process::~Process()
{
}

Process2::~Process2()
{
}

void Process::Start(const PathName & fileName, const string & arguments, FILE * pFileStandardInput, FILE ** ppFileStandardInput, FILE ** ppFileStandardOutput, FILE ** ppFileStandardError, const char * lpszWorkingDirectory)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszWorkingDirectory);

  MIKTEX_ASSERT(pFileStandardInput == nullptr || ppFileStandardInput == nullptr);

  ProcessStartInfo startinfo;

  startinfo.FileName = fileName.ToString();
  startinfo.Arguments = arguments;

  startinfo.StandardInput = pFileStandardInput;

  startinfo.RedirectStandardInput = pFileStandardInput == nullptr && ppFileStandardInput != nullptr;
  startinfo.RedirectStandardOutput = ppFileStandardOutput != nullptr;
  startinfo.RedirectStandardError = ppFileStandardError != nullptr;

  if (lpszWorkingDirectory != nullptr)
  {
    startinfo.WorkingDirectory = lpszWorkingDirectory;
  }

  unique_ptr<Process> pProcess(Process::Start(startinfo));

  if (ppFileStandardInput != nullptr)
  {
    *ppFileStandardInput = pProcess->get_StandardInput();
  }

  if (ppFileStandardOutput != nullptr)
  {
    *ppFileStandardOutput = pProcess->get_StandardOutput();
  }

  if (ppFileStandardError != nullptr)
  {
    *ppFileStandardError = pProcess->get_StandardError();
  }

  pProcess->Close();
}

bool Process::Run(const PathName & fileName, const string & arguments, IRunProcessCallback * pCallback, int * pExitCode, const char * lpszWorkingDirectory)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszWorkingDirectory);

  ProcessStartInfo startinfo;

  startinfo.FileName = fileName.ToString();
  startinfo.Arguments = arguments;

  startinfo.StandardInput = nullptr;
  startinfo.RedirectStandardInput = false;
  startinfo.RedirectStandardOutput = pCallback != nullptr;
  startinfo.RedirectStandardError = false;

  if (lpszWorkingDirectory != nullptr)
  {
    startinfo.WorkingDirectory = lpszWorkingDirectory;
  }

  unique_ptr<Process> pProcess(Process::Start(startinfo));

  if (pCallback != nullptr)
  {
    SessionImpl::GetSession()->trace_process->WriteLine("core", T_("start reading the pipe"));
    const size_t CHUNK_SIZE = 64;
    char buf[CHUNK_SIZE];
    bool cancelled = false;
    FileStream stdoutStream(pProcess->get_StandardOutput());
    size_t total = 0;
    while (!cancelled && feof(stdoutStream.Get()) == 0)
    {
      size_t n = fread(buf, 1, CHUNK_SIZE, stdoutStream.Get());
      int err = ferror(stdoutStream.Get());
      if (err != 0 && err != EPIPE)
      {
	MIKTEX_FATAL_CRT_ERROR_2("fread", "processFileName", fileName.ToString(), "processArguments", arguments);
      }
      // pass output to caller
      total += n;
      cancelled = !pCallback->OnProcessOutput(buf, n);
    }
    SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("read %u bytes from the pipe"), static_cast<unsigned>(total));
  }

  // wait for the process to finish
  pProcess->WaitForExit();

  // get the exit code & close process
  int exitCode = pProcess->get_ExitCode();
  pProcess->Close();
  if (pExitCode != nullptr)
  {
    *pExitCode = exitCode;
    return true;
  }
  else if (exitCode == 0)
  {
    return true;
  }
  else
  {
    SessionImpl::GetSession()->trace_error->WriteFormattedLine("core", T_("%s returned with exit code %d"), Q_(fileName), static_cast<int>(exitCode));
    return false;
  }
}

void Process::Run(const PathName & fileName, const string & arguments)
{
  Process::Run(fileName, arguments, nullptr);
}

void Process::Run(const PathName & fileName, const string & arguments, IRunProcessCallback * pCallback)
{
  int exitCode;
  if (!Run(fileName, arguments, pCallback, &exitCode, nullptr) || exitCode != 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("The executed process did not succeed."), "fileName", fileName.ToString(), "arguments", arguments, "exitCode", std::to_string(exitCode));
  }
}

bool Process::ExecuteSystemCommand(const string & commandLine)
{
  return ExecuteSystemCommand(commandLine, nullptr, nullptr, nullptr);
}

bool Process::ExecuteSystemCommand(const string & commandLine, int * pExitCode)
{
  return ExecuteSystemCommand(commandLine, pExitCode, nullptr, nullptr);
}

vector<string> Process2::GetInvokerNames()
{
  vector<string> result;
  unique_ptr<Process2> pProcess(Process2::GetCurrentProcess());
  unique_ptr<Process2> pParentProcess(pProcess->get_Parent());
  const int maxLevels = 3;
  int level = 0;
  for (int level = 0; pParentProcess.get() != nullptr && level < maxLevels; ++level)
  {
    result.push_back(pParentProcess->get_ProcessName());
    pParentProcess.reset(pParentProcess->get_Parent());
  }
  if (pParentProcess.get() != nullptr)
  {
    result.push_back("...");
  }
  reverse(result.begin(), result.end());
  return result;
}
