/* miktex/Core/Process.h:                               -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#pragma once

#if !defined(B7FD926CACF346B0BA927D19810BDA19)
#define B7FD926CACF346B0BA927D19810BDA19

#include <miktex/Core/config.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Run process callback interface.
class MIKTEXNOVTABLE IRunProcessCallback
{
  /// Output function. Called by the `Process` object if new output text
  /// is available.
  /// @param output Output text bytes.
  /// @param n Number of output text bytes.
  /// @return Returns `true`, if the `Process` object shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void* output, std::size_t n) = 0;

#if 0
  /// Error function. Called by the Process object if new error text
  /// is available.
  /// @param error Error text bytes.
  /// @param n Number of error text bytes.
  /// @return Returns `true`, if the `Process` object shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProcessError(const void* error, std::size_t n) = 0;
#endif
};

/// A callback interface to save process output.
template<std::size_t MaxStdoutSize_ = 1024, std::size_t ExpectedStdoutSize_ = 1024> class ProcessOutput :
  public IRunProcessCallback
{
public:
  ProcessOutput() :
    stdoutBytes(ExpectedStdoutSize_)
  {
  }

public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* bytes, std::size_t nBytes) override
  {
#if defined(_MSC_VER)
#pragma push_macro("min")
#undef min
#endif
    std::size_t n = std::min(nBytes, MaxStdoutSize_ - stdoutOffset);
#if defined(MIKTEX_WINDOWS)
#pragma pop_macro("min")
#endif
    if (n > 0)
    {
      stdoutBytes.reserve(stdoutOffset + n);
      std::copy(reinterpret_cast<const uint8_t*>(bytes), reinterpret_cast<const uint8_t*>(bytes) + n, stdoutBytes.data() + stdoutOffset);
      stdoutOffset += n;
    }
    return true;
  }

  /// Gets the saved process output.
  /// @return Returns the saved process output as a byte array.
public:
  std::vector<uint8_t> GetStandardOutput() const
  {
    return stdoutBytes;
  }

  /// Gets the saved process output.
  /// @return Returns the saved process output as a string.
public:
  std::string StdoutToString() const
  {
    // FIXME: assume UTF-8
    std::string result;
    result.reserve(stdoutOffset);
    result.assign(reinterpret_cast<const char*>(stdoutBytes.data()), reinterpret_cast<const char*>(stdoutBytes.data() + stdoutOffset));
    return result;
  }

private:
  std::vector<uint8_t> stdoutBytes;

private:
  std::size_t stdoutOffset = 0;
};

/// Process start options
struct ProcessStartInfo
{
  /// Arguments to be passed to the process.
  /// Thr first argument being the process name.
  std::vector<std::string> Arguments;

  /// File system path to the executable file.
  std::string FileName;

#if defined(MIKTEX_WINDOWS)
  /// Pointer to the output `FILE` for `stderr` redirection.
  FILE* StandardError = nullptr;
#endif

  /// Pointer to the input `FILE` stream for `stdin` redirection.
  FILE* StandardInput = nullptr;

#if defined(MIKTEX_WINDOWS)
  /// Pointer to the output `FILE` for `stdout` redirection.
  FILE* StandardOutput = nullptr;
#endif

  /// Indicates whether error output shall be written to a pipe.
  bool RedirectStandardError = false;

  /// Indicates whether input shall be read from a pipe.
  bool RedirectStandardInput = false;

  /// Indicates whether output shall be written to a pipe.
  bool RedirectStandardOutput = false;

  /// Working directory for the process.
  std::string WorkingDirectory;
  
  /// Indicates whether the process should be detached.
  bool Daemonize = false;

  ProcessStartInfo()
  {
  }

  ProcessStartInfo(const PathName& fileName) :
    FileName(fileName.ToString())
  {
  }
};

/// Process status.
enum class ProcessStatus
{
  None,
  Runnable,
  Sleeping,
  Stoped,
  Zombie,
  Other
};

/// Process exit status.
enum class ProcessExitStatus
{
  None,
  Exited,
  Signaled,
  Stopped
};

/// Process information.
struct ProcessInfo
{
  std::string name;
  ProcessStatus status = ProcessStatus::None;
  int parent = -1;
};

/// An instance of this class manages a child process.
class MIKTEXNOVTABLE Process
{
public:
  virtual MIKTEXTHISCALL ~Process() noexcept = 0;

  /// Gets the redirected `stdin`.
  /// @return Returns a pointer to an input `FILE` object. Returns `nullptr`, if
  /// the input was not redirected.
public:
  virtual FILE* MIKTEXTHISCALL get_StandardInput() = 0;

  /// Gets the redirected `stdout`.
  /// @return Returns a pointer to an output `FILE` object. Returns `nullptr`, if
  /// the output was not redirected.
public:
  virtual FILE* MIKTEXTHISCALL get_StandardOutput() = 0;

  /// Gets the redirected `stderr`.
  /// @return Returns a pointer to an output `FILE` object. Returns `nullptr`, if
  /// the error output was not redirected.
public:
  virtual FILE* MIKTEXTHISCALL get_StandardError() = 0;

  /// Waits for the process to finish.
public:
  virtual void MIKTEXTHISCALL WaitForExit() = 0;

  /// Waits for the process to finish.
  /// @param milliseconds The maximum time to wait.
  /// @return Returns `true`, if the process has finished.
public:
  virtual bool MIKTEXTHISCALL WaitForExit(int milliseconds) = 0;

  /// Gets the exit status of the process.
  /// @return Returns the exit status.
public:
  virtual ProcessExitStatus MIKTEXTHISCALL get_ExitStatus() const = 0;

  /// Gets the exit code of the process.
  /// @return Returns the exit code.
public:
  virtual int MIKTEXTHISCALL get_ExitCode() const = 0;

  /// Gets the MiKTeX exception thrown by the process.
  /// @param[out] ex The MiKTeX exception.
public:
  virtual bool MIKTEXTHISCALL get_Exception(MiKTeX::Core::MiKTeXException& ex) const = 0;

  /// Frees all resources associated with this `Process` object.
public:
  virtual void MIKTEXTHISCALL Close() = 0;

  /// Gets the operating system identification of this process.
  /// @return Returns the OS ID.
public:
  virtual int MIKTEXTHISCALL GetSystemId() = 0;

  /// Gets the parent process of this process.
  /// @return Returns a smart pointer to the parent process.
public:
  virtual std::unique_ptr<Process> MIKTEXTHISCALL get_Parent() = 0;

  /// Gets the name of this process.
  /// @return Returns the process name.
public:
  virtual std::string MIKTEXTHISCALL get_ProcessName() = 0;

  /// Gets information about this process.
  /// @return Returns the process information.
public:
  virtual ProcessInfo MIKTEXTHISCALL GetProcessInfo() = 0;

  /// Gets the current process.
  /// @return Returns s smart pointer to the current process.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<Process>) GetCurrentProcess();

  /// Gets a process.
  /// @param systemId Identifies the process.
  /// @return Returns a smart pointer to the requested process.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<Process>) GetProcess(int systemId);

  /// Gets the invoker chain for this process.
  /// @return Returns the invoker chain (parent process names) for this process.
public:
  static MIKTEXCORECEEAPI(std::vector<std::string>) GetInvokerNames();

  /// Starts the system shell to execute a command.
  /// @param commandLine The command (and arguments) to be executed.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<Process>) StartSystemCommand(const std::string& commandLine, FILE** ppFileStandardInput, FILE** ppFileStandardOutput);

  /// Starts the system shell to execute a command.
  /// @param commandLine The command (and arguments) to be executed.
public:
  static void StartSystemCommand(const std::string& commandLine)
  {
    auto process = StartSystemCommand(commandLine, nullptr, nullptr);
    process->Close();
  }

  /// Runs the system shell to execute a command.
  /// @param commandLine The command (and arguments) to be executed.
  /// @return Returns true, if the command exited successfull.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string& commandLine);

  /// Runs the system shell to execute a command.
  /// @param commandLine The command (and arguments) to be executed.
  /// @param[out] exitCode The exit code of the command.
  /// @return Returns `true`, if the process exited successfully, or
  /// if `exitCode` isn't `nullptr`.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string& commandLine, int* exitCode);

  /// Runs the system shell to execute a command.
  /// @param commandLine The command (and arguments) to be executed.
  /// @param[out] exitCode The exit code of the command.
  /// @param callback The pointer to a callback interface.
  /// @param workingDirectory Working directory for the command.
  /// @return Returns `true`, if the process exited successfully, or
  /// if `exitCode` isn't `nullptr`.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string& commandLine, int* exitCode, IRunProcessCallback* callback, const char* workingDirectory);

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
public:
  static MIKTEXCORECEEAPI(void) Run(const PathName& fileName, const std::vector<std::string>& arguments);

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
public:
  static void Run(const PathName& fileName)
  {
    Run(fileName, std::vector<std::string>{ fileName.ToString() });
  }

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
  /// @param callback Pointer to a callback interface.
public:
  static MIKTEXCORECEEAPI(void) Run(const PathName& fileName, const std::vector<std::string>& arguments, IRunProcessCallback* callback);

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
  /// @param callback The pointer to a callback interface.
  /// @param[out] exitCode The exit code of the process.
  /// @param[out] miktexException The MiKTeX exception thrown by the process.
  /// @param workingDirectory Working directory for the command.
  /// @return Returns `true`, if the process exited successfully, or
  /// if `exitCode` isn't `nullptr`.
public:
  static MIKTEXCORECEEAPI(bool) Run(const PathName& fileName, const std::vector<std::string>& arguments, IRunProcessCallback* callback, int* exitCode, MiKTeXException* miktexException, const char* workingDirectory);

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
  /// @param callback The pointer to a callback interface.
  /// @param[out] exitCode The exit code of the process.
  /// @param[out] miktexException The MiKTeX exception thrown by the process.
  /// @param workingDirectory Working directory for the command.
  /// @return Returns `true`, if the process exited successfully, or
  /// if `exitCode` isn't `nullptr`.
public:
  static MIKTEXCORECEEAPI(bool) Run(const PathName& fileName, const std::vector<std::string>& arguments, std::function<bool(const void*, std::size_t)> callback, int* exitCode, MiKTeXException* miktexException, const char* workingDirectory);

  /// Executes a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
  /// @param callback The pointer to a callback interface.
  /// @param[out] exitCode The exit code of the process.
  /// @param workingDirectory Working directory for the command.
  /// @return Returns `true`, if the process exited successfully, or
  /// if `exitCode` isn't `nullptr`.
public:
  static bool Run(const PathName& fileName, const std::vector<std::string>& arguments, IRunProcessCallback* callback, int* exitCode, const char* workingDirectory)
  {
    return Run(fileName, arguments, callback, exitCode, nullptr, workingDirectory);
  }

  /// Starts a process.
  /// @param startInfo The process start options.
  /// @return Returns a smart pointer to the process.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<Process>) Start(const ProcessStartInfo& startinfo);

  /// Starts a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
  /// @param pFileStandardInput Optional pointer to the input `FILE` stream for `stdin` redirection.
  /// @param[out] ppFileStandardInput An optional pointer to the input `FILE` stream for `stdin` redirection.
  /// @param[out] ppFileStandardOutput An optional pointer to the output `FILE` for `stdout` redirection.
  /// @param[out] ppFileStandardError An optional pointer to the error output `FILE` for `stderr` redirection.
  /// @param workingDirectory Optional working directory for the command.
public:
  static MIKTEXCORECEEAPI(void) Start(const PathName& fileName, const std::vector<std::string>& arguments, FILE* pFileStandardInput, FILE** ppFileStandardInput, FILE** ppFileStandardOutput, FILE** ppFileStandardError, const char* workingDirectory);

  /// Starts a process.
  /// @param fileName The name of an executable file to run in the process.
public:
  static void Start(const PathName& fileName)
  {
    Start(fileName, std::vector<std::string>{ fileName.GetFileNameWithoutExtension().ToString() }, nullptr, nullptr, nullptr, nullptr, nullptr);
  }

  /// Starts a process.
  /// @param fileName The name of an executable file to run in the process.
  /// @param arguments The command-line arguments to pass when starting
  /// the process.
public:
  static void Start(const PathName& fileName, const std::vector<std::string>& arguments)
  {
    Start(fileName, arguments, nullptr, nullptr, nullptr, nullptr, nullptr);
  }

public:
  static MIKTEXCORECEEAPI(void) Overlay(const PathName& fileName, const std::vector<std::string>& arguments);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
