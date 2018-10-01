/* process.cpp: executing secondary processes

   Copyright (C) 1996-2018 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "internal.h"

#include "miktex/Core/CommandLineBuilder.h"
#include "miktex/Core/Process.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

Process::~Process() noexcept
{
}

void Process::Start(const PathName& fileName, const vector<string>& arguments, FILE* pFileStandardInput, FILE** ppFileStandardInput, FILE** ppFileStandardOutput, FILE** ppFileStandardError, const char* workingDirectory)
{
  MIKTEX_ASSERT_STRING_OR_NIL(workingDirectory);

  MIKTEX_ASSERT(pFileStandardInput == nullptr || ppFileStandardInput == nullptr);

  ProcessStartInfo startinfo;

  startinfo.FileName = fileName.ToString();
  startinfo.Arguments = arguments;

  startinfo.StandardInput = pFileStandardInput;

  startinfo.RedirectStandardInput = pFileStandardInput == nullptr && ppFileStandardInput != nullptr;
  startinfo.RedirectStandardOutput = ppFileStandardOutput != nullptr;
  startinfo.RedirectStandardError = ppFileStandardError != nullptr;

  if (workingDirectory != nullptr)
  {
    startinfo.WorkingDirectory = workingDirectory;
  }

  unique_ptr<Process> process(Process::Start(startinfo));

  if (ppFileStandardInput != nullptr)
  {
    *ppFileStandardInput = process->get_StandardInput();
  }

  if (ppFileStandardOutput != nullptr)
  {
    *ppFileStandardOutput = process->get_StandardOutput();
  }

  if (ppFileStandardError != nullptr)
  {
    *ppFileStandardError = process->get_StandardError();
  }

  process->Close();
}

bool Process::Run(const PathName& fileName, const vector<string>& arguments, function<bool(const void*, size_t)> callback, int* exitCode, MiKTeXException* miktexException, const char* workingDirectory)
{
  MIKTEX_ASSERT_STRING_OR_NIL(workingDirectory);

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  ProcessStartInfo startinfo;

  startinfo.FileName = fileName.ToString();
  startinfo.Arguments = arguments;

  startinfo.StandardInput = nullptr;
  startinfo.RedirectStandardInput = false;
  startinfo.RedirectStandardOutput = callback ? true : false;
  startinfo.RedirectStandardError = false;

  if (workingDirectory != nullptr)
  {
    startinfo.WorkingDirectory = workingDirectory;
  }

  unique_ptr<Process> process(Process::Start(startinfo));

  if (callback)
  {
    if (session != nullptr)
    {
      session->trace_process->WriteLine("core", "start reading the pipe");
    }
    const size_t CHUNK_SIZE = 64;
    char buf[CHUNK_SIZE];
    bool cancelled = false;
    FileStream stdoutStream(process->get_StandardOutput());
    size_t total = 0;
    while (!cancelled && feof(stdoutStream.Get()) == 0)
    {
      size_t n = fread(buf, 1, CHUNK_SIZE, stdoutStream.Get());
      int err = ferror(stdoutStream.Get());
      if (err != 0 && err != EPIPE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("fread", "processFileName", fileName.ToString());
      }
      // pass output to caller
      total += n;
      cancelled = !callback(buf, n);
    }
    if (session != nullptr)
    {
      session->trace_process->WriteFormattedLine("core", "read %u bytes from the pipe", static_cast<unsigned>(total));
    }
  }

  // wait for the process to finish
  process->WaitForExit();

  // get the exit code & close process
  int processExitCode = process->get_ExitCode();
  MiKTeXException processException;
  bool haveException = process->get_Exception(processException);
  process->Close();

  if (processExitCode != 0 && miktexException != nullptr)
  {
    if (haveException)
    {
      *miktexException = processException;
    }
    else
    {
      *miktexException = MiKTeXException(
        fileName.GetFileName().ToDisplayString(),
        T_("The executed process did not succeed."),
        MiKTeXException::KVMAP(
          "fileName", fileName.ToDisplayString(),
          "exitCode", std::to_string(processExitCode)),
        SourceLocation());
    }
  }

  if (exitCode != nullptr)
  {
    *exitCode = processExitCode;
    return true;
  }
  else if (processExitCode == 0)
  {
    return true;
  }
  else
  {
    if (session != nullptr)
    {
      session->trace_error->WriteFormattedLine("core", "%s returned with exit code %d", Q_(fileName), static_cast<int>(processExitCode));
    }
    return false;
  }
}

bool Process::Run(const PathName& fileName, const vector<string>& arguments, IRunProcessCallback* callback, int* exitCode, MiKTeXException* miktexException, const char* workingDirectory)
{
  function<bool(const void*, size_t)> fcallback;
  if (callback != nullptr)
  {
    fcallback = [callback](const void* output, size_t n) { return callback->OnProcessOutput(output, n); };
  }
  return Run(fileName, arguments, fcallback, exitCode, miktexException, workingDirectory);
}

void Process::Run(const PathName& fileName, const vector<string>& arguments)
{
  Process::Run(fileName, arguments, nullptr);
}

void Process::Run(const PathName& fileName, const vector<string>& arguments, IRunProcessCallback* callback)
{
  int exitCode;
  MiKTeXException miktexException;
  if (!Run(fileName, arguments, callback, &exitCode, &miktexException, nullptr) || exitCode != 0)
  {
    throw miktexException;
  }
}

bool Process::ExecuteSystemCommand(const string& commandLine)
{
  return ExecuteSystemCommand(commandLine, nullptr, nullptr, nullptr);
}

bool Process::ExecuteSystemCommand(const string& commandLine, int* exitCode)
{
  return ExecuteSystemCommand(commandLine, exitCode, nullptr, nullptr);
}

vector<string> Process::GetInvokerNames()
{
  vector<string> result;
  unique_ptr<Process> process(Process::GetCurrentProcess());
  unique_ptr<Process> parentProcess(process->get_Parent());
  const int maxLevels = 3;
  for (int level = 0; parentProcess.get() != nullptr && level < maxLevels; ++level)
  {
    result.push_back(parentProcess->get_ProcessName());
    parentProcess = parentProcess->get_Parent();
  }
  if (parentProcess.get() != nullptr)
  {
    result.push_back("...");
  }
  reverse(result.begin(), result.end());
  return result;
}
