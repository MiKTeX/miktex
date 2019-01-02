/* winProcess.cpp: executing secondary processes

   Copyright (C) 1996-2019 Christian Schenk

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

#include <Windows.h>
#include <Tlhelp32.h>

#include <fcntl.h>

#include <io.h>

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Environment>
#include <miktex/Core/win/winAutoResource>

#include "internal.h"

#include "winProcess.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<Process> Process::Start(const ProcessStartInfo& startinfo)
{
  return make_unique<winProcess>(startinfo);
}

#if defined(NDEBUG)
#  define TRACEREDIR 1
#else
#  define TRACEREDIR 1
#endif

#define MERGE_STDOUT_STDERR 1

void winProcess::Create()
{
  MIKTEX_EXPECT(!startinfo.FileName.empty());

  PathName fileName;

  if (Utils::IsAbsolutePath(startinfo.FileName))
  {
    fileName = startinfo.FileName;
  }
  else
  {
    wchar_t* lpszFilePart = nullptr;
    wchar_t szFileName[_MAX_PATH];
    if (SearchPathW(nullptr, UW_(startinfo.FileName), L".exe", _MAX_PATH, szFileName, &lpszFilePart) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("SearchPath", "fileName", startinfo.FileName);
    }
    fileName = szFileName;
  }

  CommandLineBuilder commandLine;
  commandLine.SetQuotingConvention(QuotingConvention::Whitespace);
  commandLine.AppendArguments(startinfo.Arguments.empty() ? vector<string>{ PathName(startinfo.FileName).GetFileNameWithoutExtension().ToString() } : startinfo.Arguments);

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
    shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

    // redirect stdout (and possibly stderr)
    if (startinfo.StandardOutput != nullptr)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stdout to a stream");
      }
#endif
      int fd = _fileno(startinfo.StandardOutput);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      HANDLE hStdout = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStdout == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      if (!DuplicateHandle(hCurrentProcess, hStdout, hCurrentProcess, &hChildStdout, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }
    else if (startinfo.RedirectStandardOutput)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stdout to a pipe");
      }
#endif
      // create stdout pipe
      AutoHANDLE hStdoutRd;
      if (!CreatePipe(&hStdoutRd, &hChildStdout, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      // duplicate the read end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStdoutRd.Get(), hCurrentProcess, &standardOutput, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }

    // redirect stderr
    if (startinfo.StandardError != nullptr)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stderr to a stream");
      }
#endif
      int fd = _fileno(startinfo.StandardError);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      HANDLE hStderr = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStderr == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      if (!DuplicateHandle(hCurrentProcess, hStderr, hCurrentProcess, &hChildStderr, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }
    else if (startinfo.RedirectStandardError)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stderr to a pipe");
      }
#endif
      // create child stderr pipe
      AutoHANDLE hStderrRd;
      if (!CreatePipe(&hStderrRd, &hChildStderr, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      // duplicate the read end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStderrRd.Get(), hCurrentProcess, &standardError, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }
#if MERGE_STDOUT_STDERR
    else if (hChildStdout != INVALID_HANDLE_VALUE)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "make child stderr = child stdout");
      }
#endif
      // make child stderr = child stdout
      if (!DuplicateHandle(hCurrentProcess, hChildStdout, hCurrentProcess, &hChildStderr, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }
#endif

    // redirect stdin
    if (startinfo.StandardInput != nullptr)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stdin to a stream");
      }
#endif
      int fd = _fileno(startinfo.StandardInput);
      if (fd < 0)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_fileno", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      HANDLE hStdin = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
      if (hStdin == INVALID_HANDLE_VALUE)
      {
        MIKTEX_FATAL_CRT_ERROR_2("_get_osfhandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      if (!DuplicateHandle(hCurrentProcess, hStdin, hCurrentProcess, &hChildStdin, 0, TRUE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }
    else if (startinfo.RedirectStandardInput)
    {
#if TRACEREDIR
      if (session != nullptr)
      {
        session->trace_process->WriteFormattedLine("core", "redirecting stdin to a pipe");
      }
#endif
      // create child stdin pipe
      AutoHANDLE hStdinWr;
      if (!CreatePipe(&hChildStdin, &hStdinWr, const_cast<LPSECURITY_ATTRIBUTES>(&SAPIPE), PIPE_BUF_SIZE))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreatePipe", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
      // duplicate the write end of the pipe
      if (!DuplicateHandle(hCurrentProcess, hStdinWr.Get(), hCurrentProcess, &standardInput, 0, FALSE, DUPLICATE_SAME_ACCESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("DuplicateHandle", "processFileName", startinfo.FileName, "commandLine", commandLine.ToString());
      }
    }

    // set child handles
    STARTUPINFOW siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(siStartInfo);
    if (hChildStdin != INVALID_HANDLE_VALUE || hChildStdout != INVALID_HANDLE_VALUE || hChildStderr != INVALID_HANDLE_VALUE)
    {
      siStartInfo.dwFlags = STARTF_USESTDHANDLES;
      siStartInfo.hStdInput = hChildStdin != INVALID_HANDLE_VALUE ? hChildStdin : GetStdHandle(STD_INPUT_HANDLE);
      siStartInfo.hStdOutput = hChildStdout != INVALID_HANDLE_VALUE ? hChildStdout : GetStdHandle(STD_OUTPUT_HANDLE);
      siStartInfo.hStdError = hChildStderr != INVALID_HANDLE_VALUE ? hChildStderr : GetStdHandle(STD_ERROR_HANDLE);
    }

    DWORD creationFlags = 0;

    // don't open a window if both stdout & stderr are redirected or if we are shutting down
    if (hChildStdout != INVALID_HANDLE_VALUE && hChildStderr != INVALID_HANDLE_VALUE || session == nullptr)
    {
      creationFlags |= CREATE_NO_WINDOW;
    }

    // set environment variables
    if (session != nullptr)
    {
      // FIXME
      session->SetEnvironmentVariables();
    }

    // start child process
    if (session != nullptr)
    {
      session->trace_process->WriteFormattedLine("core", "start process: %s", commandLine.ToString().c_str());
    }

    tmpFile = TemporaryFile::Create();
    tmpEnv.Set(MIKTEX_ENV_EXCEPTION_PATH, tmpFile->GetPathName().ToString());

    if (!CreateProcessW(UW_(fileName.GetData()), UW_(commandLine.ToString()), nullptr, nullptr, TRUE, creationFlags, nullptr, startinfo.WorkingDirectory.empty() ? nullptr : UW_(startinfo.WorkingDirectory), &siStartInfo, &processInformation))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CreateProcess", "fileName", startinfo.FileName, "commandLine", commandLine.ToString());
    }
    processStarted = true;
  }

  catch (const exception&)
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

winProcess::winProcess(const ProcessStartInfo& startinfo) :
  startinfo(startinfo)
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
  catch (const exception&)
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
    if (processInformation.hThread != nullptr)
    {
      CloseHandle(processInformation.hThread);
    }
  }
  if (tmpFile != nullptr)
  {
    tmpEnv.Restore();
    tmpFile->Delete();
    tmpFile = nullptr;
  }
}

FILE* winProcess::get_StandardInput()
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

FILE* winProcess::get_StandardOutput()
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

FILE* winProcess::get_StandardError()
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
  return WaitForSingleObject(processInformation.hProcess, static_cast<DWORD>(milliseconds)) == WAIT_OBJECT_0;
}

int winProcess::get_ExitCode() const
{
  DWORD exitCode;
  if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetExitCodeProcess");
  }
  MIKTEX_EXPECT(exitCode != STATUS_PENDING);
  if (exitCode == STATUS_ACCESS_VIOLATION)
  {
    MIKTEX_FATAL_ERROR_2(T_("The process terminated due to an access violation."), "fileName", startinfo.FileName);
  }
  return static_cast<int>(exitCode);
}

bool winProcess::get_Exception(MiKTeXException& ex) const
{
  return MiKTeXException::Load(tmpFile->GetPathName().ToString(), ex);
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
      if (!Utils::IsAbsolutePath(path))
      {
        wchar_t* lpsz = nullptr;
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
      const wchar_t* lpszShell = L"cmd.exe";
      wchar_t* lpsz = nullptr;
      MIKTEX_EXPECT(SearchPathW(nullptr, lpszShell, nullptr, ARRAY_SIZE(szCmd), szCmd, &lpsz) != 0);
    }
  }

  return szCmd;
}

MIKTEXSTATICFUNC(vector<string>) Wrap(const string& commandLine)
{
  return vector<string> {
    FindSystemShell().ToString(),
    "/c",
    commandLine
  };
}

bool Process::ExecuteSystemCommand(const string& commandLine, int* exitCode, IRunProcessCallback* callback, const char* workingDirectory)
{
  vector<string> arguments = Wrap(commandLine);
  return Process::Run(arguments[0], arguments, callback, exitCode, workingDirectory);
}

void Process::StartSystemCommand(const string& commandLine)
{
  vector<string> arguments = Wrap(commandLine);
  Process::Start(arguments[0], arguments);
}

winProcess::winProcess()
{
  processEntry.dwSize = 0;
}

unique_ptr<Process> Process::GetCurrentProcess()
{
  HANDLE myHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
  if (myHandle == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("OpenProcess");
  }
  unique_ptr<winProcess> currentProcess = make_unique<winProcess>();
  currentProcess->processStarted = true;
  currentProcess->processInformation.hProcess = myHandle;
  currentProcess->processInformation.hThread = GetCurrentThread();
  currentProcess->processInformation.dwProcessId = GetCurrentProcessId();
  currentProcess->processInformation.dwThreadId = GetCurrentThreadId();
  return currentProcess;
}

unique_ptr<Process> Process::GetProcess(int systemId)
{
  HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, systemId);
  if (handle == nullptr)
  {
    if (GetLastError() == ERROR_ACCESS_DENIED)
    {
      throw UnauthorizedAccessException();
    }
    return nullptr;
  }
  unique_ptr<winProcess> process = make_unique<winProcess>();
  process->processStarted = true;
  process->processInformation.hProcess = handle;
  process->processInformation.hThread = nullptr;
  process->processInformation.dwProcessId = systemId;
  process->processInformation.dwThreadId = -1;
  return process;
}

bool winProcess::TryGetProcessEntry(DWORD processId, PROCESSENTRY32W& result)
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
    MIKTEX_UNEXPECTED();
  }
  return result;
}

unique_ptr<Process> winProcess::get_Parent()
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
  unique_ptr<winProcess> parent = make_unique<winProcess>();
  parent->processStarted = true;
  parent->processInformation.hProcess = parentProcessHandle;
  parent->processInformation.dwProcessId = processEntry.th32ParentProcessID;
  parent->processInformation.hThread = nullptr;
  parent->processInformation.dwThreadId = 0;
  return parent;
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
  return exePath.GetFileNameWithoutExtension().GetData();
}

int winProcess::GetSystemId()
{
  return processInformation.dwProcessId;
}

ProcessInfo winProcess::GetProcessInfo()
{
  ProcessInfo processInfo;
  processInfo.name = get_ProcessName();
  processInfo.parent = get_Parent()->GetSystemId();
  processInfo.status = ProcessStatus::Runnable;
  return processInfo;
}
