/* winProcess.cpp: executing secondary processes

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

#include <Windows.h>
#include <Tlhelp32.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <fcntl.h>

#include <io.h>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Environment>
#include <miktex/Core/win/winAutoResource>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

#include "winProcess.h"
#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
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

  if (PathNameUtil::IsAbsolutePath(startinfo.FileName))
  {
    fileName = startinfo.FileName;
  }
  else
  {
    CharBuffer<wchar_t, BufferSizes::MaxPath> foundFileName;
    bool done = false;
    int rounds = 0;
    do
    {
      wchar_t* lpszFilePart = nullptr;
      DWORD n = SearchPathW(nullptr, UW_(startinfo.FileName), L".exe", foundFileName.GetCapacity(), foundFileName.GetData(), &lpszFilePart);
      if (n == 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("SearchPath", "fileName", startinfo.FileName);
      }
      done = n < foundFileName.GetCapacity();
      if (!done)
      {
        if (rounds > 0)
        {
          BUF_TOO_SMALL();
        }
        foundFileName.Reserve(n);
      }
      rounds++;
    } while (!done);
    fileName = foundFileName.GetData();
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

  auto trace_process = TraceStream::Open(MIKTEX_TRACE_PROCESS);

  try
  {
    // redirect stdout (and possibly stderr)
    if (startinfo.StandardOutput != nullptr)
    {
#if TRACEREDIR
      trace_process->WriteLine("core", "redirecting stdout to a stream");
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
      trace_process->WriteLine("core", "redirecting stdout to a pipe");
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
      trace_process->WriteLine("core", "redirecting stderr to a stream");
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
      trace_process->WriteLine("core", "redirecting stderr to a pipe");
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
      trace_process->WriteLine("core", "make child stderr = child stdout");
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
      trace_process->WriteLine("core", "redirecting stdin to a stream");
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
      trace_process->WriteLine("core", "redirecting stdin to a pipe");
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

    DWORD creationFlags = CREATE_UNICODE_ENVIRONMENT;

    // don't open a window if both stdout & stderr are redirected or if we are shutting down
    if (hChildStdout != INVALID_HANDLE_VALUE && hChildStderr != INVALID_HANDLE_VALUE)
    {
      creationFlags |= CREATE_NO_WINDOW;
    }

    // start child process
    trace_process->WriteLine("core", TraceLevel::Info, fmt::format("start process: {0}", commandLine));

    shared_ptr<SessionImpl> session = SESSION_IMPL();

    // create environment map
    unordered_map<string, string> envMap = session->CreateChildEnvironment(!startinfo.WorkingDirectory.empty());

    tmpFile = TemporaryFile::Create();
    envMap[MIKTEX_ENV_EXCEPTION_PATH] = tmpFile->GetPathName().ToString();

    // create environment strings
    size_t envSize = 0;
    for (const auto& p : envMap)
    {
      envSize += p.first.length() + 1 + p.second.length() + 1;
    }
    envSize += 1;
    wchar_t* environmentStrings = new wchar_t[envSize];
    MIKTEX_AUTO(delete[]environmentStrings);
    size_t stringIdx = 0;
    for (const auto& p : envMap)
    {
      wstring s = fmt::format(L"{}={}", UW_(p.first), UW_(p.second));
      wcscpy_s(environmentStrings + stringIdx, envSize - stringIdx, s.c_str());
      stringIdx += s.length() + 1;
    }
    environmentStrings[stringIdx] = wchar_t();

    session->UnloadFilenameDatabase();

    if (!CreateProcessW(UW_(fileName.GetData()), UW_(commandLine.ToString()), nullptr, nullptr, TRUE, creationFlags, environmentStrings, startinfo.WorkingDirectory.empty() ? nullptr : UW_(startinfo.WorkingDirectory), &siStartInfo, &processInformation))
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

ProcessExitStatus winProcess::get_ExitStatus() const
{
  DWORD exitCode;
  if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetExitCodeProcess");
  }
  MIKTEX_EXPECT(exitCode != STATUS_PENDING);
  return exitCode < 128 ? ProcessExitStatus::Exited : ProcessExitStatus::Other;
}

int winProcess::get_ExitCode() const
{
  DWORD exitCode;
  if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetExitCodeProcess");
  }
  MIKTEX_EXPECT(exitCode != STATUS_PENDING);
  if (exitCode > 127)
  {
    MIKTEX_FATAL_ERROR_2(T_("The child process exited abnormally."), "fileName", startinfo.FileName, "exitCode", std::to_string(exitCode));
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
      if (!PathNameUtil::IsAbsolutePath(path))
      {
        wchar_t* lpsz = nullptr;
        if (SearchPathW(nullptr, PathName(path).ToWideCharString().c_str(), nullptr, ARRAY_SIZE(szCmd), szCmd, &lpsz) == 0)
        {
          szCmd[0] = 0;
        }
      }
      else if (File::Exists(PathName(path)))
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

  return PathName(szCmd);
}

MIKTEXSTATICFUNC(vector<string>) Wrap(const string& commandLine)
{
  return vector<string> {
    FindSystemShell().ToString(),
    "/c",
    commandLine
  };
}

constexpr const char* PREFIX = "//?/";
constexpr int PREFIX_LENGTH = 4;

bool Process::ExecuteSystemCommand(const string& commandLine, int* exitCode, IRunProcessCallback* callback, const char* workingDirectory)
{
  PathName workingDirectoryNonUNC;
  if (workingDirectory != nullptr && PathName::Compare(workingDirectory, PREFIX, PREFIX_LENGTH) == 0)
  {
    workingDirectoryNonUNC = &workingDirectory[PREFIX_LENGTH];
  }
  else if (workingDirectory == nullptr)
  {
    PathName cwd;
    cwd.SetToCurrentDirectory();
    if (PathName::Compare(cwd, PathName(PREFIX), PREFIX_LENGTH) == 0)
    {
      workingDirectoryNonUNC = &cwd[PREFIX_LENGTH];
    }
  }
  if (!workingDirectoryNonUNC.Empty())
  {
    workingDirectory = workingDirectoryNonUNC.GetData();
  }
  vector<string> arguments = Wrap(commandLine);
  return Process::Run(PathName(arguments[0]), arguments, callback, exitCode, workingDirectory);
}

unique_ptr<Process> Process::StartSystemCommand(const string& commandLine, FILE** ppFileStandardInput, FILE** ppFileStandardOutput)
{
  const char* workingDirectory = nullptr;
  PathName workingDirectoryNonUNC;
  PathName cwd;
  cwd.SetToCurrentDirectory();
  if (PathName::Compare(cwd, PathName(PREFIX), PREFIX_LENGTH) == 0)
  {
    workingDirectoryNonUNC = &cwd[PREFIX_LENGTH];
  }
  if (!workingDirectoryNonUNC.Empty())
  {
    workingDirectory = workingDirectoryNonUNC.GetData();
  }
  vector<string> arguments = Wrap(commandLine);
  ProcessStartInfo startinfo;
  startinfo.FileName = arguments[0];
  startinfo.Arguments = arguments;
  startinfo.RedirectStandardInput = ppFileStandardInput != nullptr;
  startinfo.RedirectStandardOutput = ppFileStandardOutput != nullptr;
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
  return process;
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

void Process::Overlay(const PathName& fileName, const std::vector<std::string>& arguments)
{
  int exitCode;
  Process::Run(fileName, arguments, nullptr, &exitCode, nullptr);
  throw exitCode;
}
