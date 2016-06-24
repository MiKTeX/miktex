/* winProcess.cpp: executing secondary processes

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

#include "miktex/Core/win/winAutoResource.h"

#include "winProcess.h"
#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

// TODO: return unique_ptr<Process>
Process * Process::Start(const ProcessStartInfo & startinfo)
{
  return new winProcess(startinfo);
}

#if defined(NDEBUG)
#  define TRACEREDIR 1
#else
#  define TRACEREDIR 1
#endif

#define MERGE_STDOUT_STDERR 1

void winProcess::Create()
{
  if (startinfo.FileName.empty())
  {
    MIKTEX_UNEXPECTED();
  }

  PathName fileName;

  if (Utils::IsAbsolutePath(startinfo.FileName.c_str()))
  {
    fileName = startinfo.FileName;
  }
  else
  {
    wchar_t * lpszFilePart = nullptr;
    wchar_t szFileName[_MAX_PATH];
    if (SearchPathW(nullptr, UW_(startinfo.FileName), nullptr, _MAX_PATH, szFileName, &lpszFilePart) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("SearchPath", "fileName", startinfo.FileName);
    }
    fileName = szFileName;
  }

  // standard security attributes for pipes
  SECURITY_ATTRIBUTES const SAPIPE = {
    sizeof(SECURITY_ATTRIBUTES),
    nullptr,                    // default security descriptor
    TRUE,                       // make the handle inheritable
  };

  const unsigned long PIPE_BUF_SIZE = 0; // use default size

  HANDLE hCurrentProcess = ::GetCurrentProcess();

  // inheritable child handles
  HANDLE hChildStdin = INVALID_HANDLE_VALUE;
  HANDLE hChildStdout = INVALID_HANDLE_VALUE;
  HANDLE hChildStderr = INVALID_HANDLE_VALUE;

  try
  {
    // redirect stdout (and possibly stderr)
    if (startinfo.StandardOutput != nullptr)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stdout to a stream"));
#endif
      int fd = _fileno(startinfo.StandardOutput);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      HANDLE hStdout = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStdout == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      if (!DuplicateHandle(hCurrentProcess, hStdout, hCurrentProcess, &hChildStdout, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }
    else if (startinfo.RedirectStandardOutput)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stdout to a pipe"));
#endif
      // create stdout pipe
      AutoHANDLE hStdoutRd;
      if (!CreatePipe(&hStdoutRd, &hChildStdout, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      // duplicate the read end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStdoutRd.Get(), hCurrentProcess, &standardOutput, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }

    // redirect stderr
    if (startinfo.StandardError != nullptr)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stderr to a stream"));
#endif
      int fd = _fileno(startinfo.StandardError);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      HANDLE hStderr = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStderr == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      if (!DuplicateHandle(hCurrentProcess, hStderr, hCurrentProcess, &hChildStderr, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }
    else if (startinfo.RedirectStandardError)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stderr to a pipe"));
#endif
      // create child stderr pipe
      AutoHANDLE hStderrRd;
      if (!CreatePipe(&hStderrRd, &hChildStderr, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      // duplicate the read end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStderrRd.Get(), hCurrentProcess, &standardError, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }
#if MERGE_STDOUT_STDERR
    else if (hChildStdout != INVALID_HANDLE_VALUE)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("make child stderr = child stdout"));
#endif
      // make child stderr = child stdout
      if (!DuplicateHandle(hCurrentProcess, hChildStdout, hCurrentProcess, &hChildStderr, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }
#endif

    // redirect stdin
    if (startinfo.StandardInput != nullptr)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stdin to a stream"));
#endif
      int fd = _fileno(startinfo.StandardInput);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      HANDLE hStdin = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStdin == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      if (!DuplicateHandle(hCurrentProcess, hStdin, hCurrentProcess, &hChildStdin, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }
    else if (startinfo.RedirectStandardInput)
    {
#if TRACEREDIR
      SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("redirecting stdin to a pipe"));
#endif
      // create child stdin pipe
      AutoHANDLE hStdinWr;
      if (!CreatePipe(&hChildStdin, &hStdinWr, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
      // duplicate the write end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStdinWr.Get(), hCurrentProcess, &standardInput, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "processArguments", startinfo.Arguments);
      }
    }

    // set child handles
    STARTUPINFOW siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(siStartInfo);
    siStartInfo.dwFlags = STARTF_USESTDHANDLES;
    siStartInfo.hStdInput =
      (hChildStdin != INVALID_HANDLE_VALUE
        ? hChildStdin
        : GetStdHandle(STD_INPUT_HANDLE));
    siStartInfo.hStdOutput =
      (hChildStdout != INVALID_HANDLE_VALUE
        ? hChildStdout
        : GetStdHandle(STD_OUTPUT_HANDLE));
    siStartInfo.hStdError =
      (hChildStderr != INVALID_HANDLE_VALUE
        ? hChildStderr
        : GetStdHandle(STD_ERROR_HANDLE));

    DWORD creationFlags = 0;

    // don't open a window if both stdout & stderr are redirected
    if (hChildStdout != INVALID_HANDLE_VALUE && hChildStderr != INVALID_HANDLE_VALUE)
    {
      creationFlags |= CREATE_NO_WINDOW;
    }

    // build command-line
    string commandLine;
    bool needQuotes = (startinfo.FileName.find(' ') != string::npos);
    if (needQuotes)
    {
      commandLine = '"';
    }
    commandLine += startinfo.FileName;
    if (needQuotes)
    {
      commandLine += '"';
    }
    if (!startinfo.Arguments.empty())
    {
      commandLine += ' ';
      commandLine += startinfo.Arguments;
    }

    // set environment variables
    SessionImpl::GetSession()->SetEnvironmentVariables();

    // start child process
    SessionImpl::GetSession()->trace_process->WriteFormattedLine("core", T_("start process: %s"), commandLine.c_str());
#if 1
    // experimental
    SessionImpl::GetSession()->UnloadFilenameDatabase();
#endif
    if (!CreateProcessW(UW_(fileName.Get()), UW_(commandLine), nullptr, nullptr, TRUE, creationFlags, nullptr, startinfo.WorkingDirectory.empty() ? nullptr : UW_(startinfo.WorkingDirectory), &siStartInfo, &processInformation))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CreateProcess", "fileName", startinfo.FileName, "commandLine", commandLine);
    }
    processStarted = true;
  }

  catch (const exception &)
  {
    if (hChildStdin != INVALID_HANDLE_VALUE)
    {
      CloseHandle(hChildStdin);
    }
    if (hChildStdout != INVALID_HANDLE_VALUE)
    {
      CloseHandle(hChildStdout);
    }
    if (hChildStderr != INVALID_HANDLE_VALUE)
    {
      CloseHandle(hChildStderr);
    }
    if (standardInput != nullptr)
    {
      CloseHandle(standardInput);
    }
    if (standardOutput != nullptr)
    {
      CloseHandle(standardOutput);
    }
    if (standardError != nullptr)
    {
      CloseHandle(standardError);
    }
    throw;
  }

  if (hChildStdin != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hChildStdin);
  }

  if (hChildStdout != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hChildStdout);
  }

  if (hChildStderr != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hChildStderr);
  }
}

winProcess::winProcess(const ProcessStartInfo & startinfo)
  : startinfo(startinfo)
{
  processEntry.dwSize = 0;
  Create();
}

winProcess::~winProcess()
{
  try
  {
    Close();
  }
  catch (const exception &)
  {
  }
}

void winProcess::Close()
{
  if (standardInput != INVALID_HANDLE_VALUE)
  {
    CloseHandle(standardInput);
    standardInput = INVALID_HANDLE_VALUE;
  }
  if (standardOutput != INVALID_HANDLE_VALUE)
  {
    CloseHandle(standardOutput);
    standardOutput = INVALID_HANDLE_VALUE;
  }
  if (standardError != INVALID_HANDLE_VALUE)
  {
    CloseHandle(standardError);
    standardError = INVALID_HANDLE_VALUE;
  }
  if (processStarted)
  {
    processStarted = false;
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);
  }
}

FILE * winProcess::get_StandardInput()
{
  if (pFileStandardInput != nullptr)
  {
    return pFileStandardInput;
  }
  if (standardInput == INVALID_HANDLE_VALUE)
  {
    return nullptr;
  }
  int fd = _open_osfhandle(reinterpret_cast<intptr_t>(standardInput), O_WRONLY | O_BINARY);
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_open_osfhandle");
  }
  pFileStandardInput = FdOpen(fd, "wb");
  standardInput = INVALID_HANDLE_VALUE;
  return pFileStandardInput;
}

FILE * winProcess::get_StandardOutput()
{
  if (pFileStandardOutput != nullptr)
  {
    return pFileStandardOutput;
  }
  if (standardOutput == INVALID_HANDLE_VALUE)
  {
    return nullptr;
  }
  int fd = _open_osfhandle(reinterpret_cast<intptr_t>(standardOutput), O_RDONLY | O_BINARY);
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_open_osfhandle");
  }
  pFileStandardOutput = FdOpen(fd, "rb");
  standardOutput = INVALID_HANDLE_VALUE;
  return pFileStandardOutput;
}

FILE * winProcess::get_StandardError()
{
  if (pFileStandardError != nullptr)
  {
    return pFileStandardError;
  }
  if (standardError == INVALID_HANDLE_VALUE)
  {
    return nullptr;
  }
  int fd = _open_osfhandle(reinterpret_cast<intptr_t>(standardError), O_RDONLY | O_BINARY);
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("_open_osfhandle");
  }
  pFileStandardError = FdOpen(fd, "rb");
  standardError = INVALID_HANDLE_VALUE;
  return pFileStandardError;
}

void winProcess::WaitForExit()
{
  WaitForSingleObject(processInformation.hProcess, INFINITE);
}

bool winProcess::WaitForExit(int milliseconds)
{
  return WaitForSingleObject(processInformation.hProcess, static_cast<DWORD>(milliseconds) == WAIT_OBJECT_0);
}

int winProcess::get_ExitCode() const
{
  DWORD exitCode;
  if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetExitCodeProcess");
  }
  return static_cast<int>(exitCode);
}

MIKTEXSTATICFUNC(PathName) FindSystemShell()
{
  MIKTEXPERMANENTVAR(wchar_t) szCmd[BufferSizes::MaxPath] = { 0 };

  // locate the command interpreter
  if (szCmd[0] == 0)
  {
    string path;
    if (Utils::GetEnvironmentString("COMSPEC", path))
    {
      if (!Utils::IsAbsolutePath(path.c_str()))
      {
        wchar_t * lpsz = nullptr;
        if (SearchPathW(nullptr, PathName(path).ToWideCharString().c_str(), nullptr, ARRAY_SIZE(szCmd), szCmd, &lpsz) == 0)
        {
          szCmd[0] = 0;
        }
      }
      else if (File::Exists(path))
      {
        StringUtil::CopyString(szCmd, ARRAY_SIZE(szCmd), PathName(path).ToWideCharString().c_str());
      }
    }
    if (szCmd[0] == 0)
    {
      const wchar_t * lpszShell = L"cmd.exe";
      wchar_t * lpsz = nullptr;
      if (SearchPathW(nullptr, lpszShell, nullptr, ARRAY_SIZE(szCmd), szCmd, &lpsz) == 0)
      {
        MIKTEX_UNEXPECTED();
      }
    }
  }

  return szCmd;
}

MIKTEXSTATICFUNC(PathName) Wrap(string & arguments)
{
  PathName systemShell = FindSystemShell();
  string wrappedArguments = "/s /c \"";
  wrappedArguments += arguments;
  wrappedArguments += "\"";
  arguments = wrappedArguments;
  return systemShell;
}

/* _________________________________________________________________________

   Process::ExecuteSystemCommand

   Start cmd.exe with a command line.  Pass output (stdout & stderr)
   to caller.

   Suppose command-line is: tifftopnm "%i" | ppmtobmp -windows > "%o"

   Then we start as follows on Windows 95:

     conspawn "command /c tifftopnm \"%i\" | ppmtobmp -windows > \"%o\""

   conspawn.exe runs

     command /c tifftopnm "%i" | ppmtobmp -windows > "%o"
   _________________________________________________________________________ */

bool Process::ExecuteSystemCommand(const string & commandLine, int * pExitCode, IRunProcessCallback * pCallback, const char * lpszDirectory)
{
  string arguments(commandLine);
  PathName systemShell = Wrap(arguments);
  return Process::Run(systemShell, arguments.c_str(), pCallback, pExitCode, lpszDirectory);
}

void Process::StartSystemCommand(const string & commandLine)
{
  string arguments(commandLine);
  PathName systemShell = Wrap(arguments);
  Process::Start(systemShell, arguments.c_str());
}

winProcess::winProcess()
{
  processEntry.dwSize = 0;
}

// TODO: return unique_ptr<Process2>
Process2 * Process2::GetCurrentProcess()
{
  HANDLE myHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
  if (myHandle == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("OpenProcess");
  }
  winProcess * pCurrentProcess = new winProcess();
  pCurrentProcess->processStarted = true;
  pCurrentProcess->processInformation.hProcess = myHandle;
  pCurrentProcess->processInformation.hThread = GetCurrentThread();
  pCurrentProcess->processInformation.dwProcessId = GetCurrentProcessId();
  pCurrentProcess->processInformation.dwThreadId = GetCurrentThreadId();
  return pCurrentProcess;
}

bool winProcess::TryGetProcessEntry(DWORD processId, PROCESSENTRY32W & result)
{
  HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshotHandle == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateToolHelp32Snapshot");
  }
  AutoHANDLE autoSnapshotHandle(snapshotHandle);
  PROCESSENTRY32W processEntry;
  processEntry.dwSize = sizeof(processEntry);
  if (!Process32FirstW(snapshotHandle, &processEntry))
  {
    DWORD lastError = GetLastError();
    if (lastError == ERROR_NO_MORE_FILES)
    {
      return false;
    }
    else
    {
      MIKTEX_FATAL_WINDOWS_RESULT("Process32FirstW", lastError);
    }
  }
  do
  {
    if (processEntry.th32ProcessID == processId)
    {
      result = processEntry;
      return true;
    }
  } while (Process32NextW(snapshotHandle, &processEntry));
  DWORD lastError = GetLastError();
  if (lastError == ERROR_NO_MORE_FILES)
  {
    return false;
  }
  else
  {
    MIKTEX_FATAL_WINDOWS_RESULT("Process32NextW", lastError);
  }
}

PROCESSENTRY32W winProcess::GetProcessEntry(DWORD processId)
{
  PROCESSENTRY32W result;
  if (!TryGetProcessEntry(processId, result))
  {
    SessionImpl::GetSession()->trace_error->WriteFormattedLine("core", "error context: ID=%u", processId);
    MIKTEX_UNEXPECTED();
  }
  return result;
}

// TODO: return unique_ptr<Process2>
Process2 * winProcess::get_Parent()
{
  if (processEntry.dwSize == 0)
  {
    if (!TryGetProcessEntry(processInformation.dwProcessId, processEntry))
    {
      return nullptr;
    }
  }
  HANDLE parentProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processEntry.th32ParentProcessID);
  if (parentProcessHandle == nullptr)
  {
    return nullptr;
  }
  winProcess * pParentProcess = new winProcess();
  pParentProcess->processStarted = true;
  pParentProcess->processInformation.hProcess = parentProcessHandle;
  pParentProcess->processInformation.dwProcessId = processEntry.th32ParentProcessID;
  pParentProcess->processInformation.hThread = nullptr;
  pParentProcess->processInformation.dwThreadId = 0;
  return pParentProcess;
}

string winProcess::get_ProcessName()
{
  if (processEntry.dwSize == 0)
  {
    if (!TryGetProcessEntry(processInformation.dwProcessId, processEntry))
    {
      return "non-existing";
    }
  }
  PathName exePath(processEntry.szExeFile);
  return exePath.GetFileNameWithoutExtension().Get();
}
