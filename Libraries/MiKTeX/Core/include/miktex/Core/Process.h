/* miktex/Core/Process.h:                               -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B7FD926CACF346B0BA927D19810BDA19)
#define B7FD926CACF346B0BA927D19810BDA19

#include <miktex/Core/config.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Run process callback interface.
class MIKTEXNOVTABLE IRunProcessCallback
{
  /// Output function. Called by the Process object if new output text
  /// is available.
  /// @param pOutput Output text bytes.
  /// @param n Number of output text bytes.
  /// @return Returns true, of the Process object shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, std::size_t n) = 0;

#if 0
  /// Error function. Called by the Process object if new error text
  /// is available.
  /// @param pError Error text bytes.
  /// @param n Number of error text bytes.
  /// @return Returns true, of the Process object shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProcessError(const void * pError, std::size_t n) = 0;
#endif
};

template<std::size_t MaxStdoutSize_ = 1024> class ProcessOutput :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * bytes, std::size_t nBytes) override
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
      std::copy(reinterpret_cast<const uint8_t*>(bytes), reinterpret_cast<const uint8_t*>(bytes) + n, stdoutBytes.data() + stdoutOffset);
      stdoutOffset += n;
    }
    return true;
  }

public:
  std::vector<uint8_t> GetStandardOutput() const
  {
    return std::vector<uint8_t>(stdoutBytes.data(), stdoutBytes.data() + stdoutOffset);
  }

public:
  std::string StdoutToString()
  {
    // FIXME: assume UTF-8
    std::string result;
    result.reserve(stdoutOffset);
    result.assign(reinterpret_cast<const char*>(stdoutBytes.data()), reinterpret_cast<const char*>(stdoutBytes.data() + stdoutOffset));
    return result;
  }

private:
  std::array<uint8_t, MaxStdoutSize_> stdoutBytes;

private:
  std::size_t stdoutOffset = 0;
};

/// Proces start info struct.
struct ProcessStartInfo
{
  /// Arguments to be passed to the process.
  std::string Arguments;

  /// Path name to be executed.
  std::string FileName;

#if defined(MIKTEX_WINDOWS)
  /// Output file stream for stderr redirection.
  FILE * StandardError = nullptr;
#endif

  /// Input file stream for stdin redirection.
  FILE * StandardInput = nullptr;

#if defined(MIKTEX_WINDOWS)
  /// Output file stream for stdout redirection.
  FILE * StandardOutput = nullptr;
#endif

  /// Indicates whether error output shall be written to a pipe.
  bool RedirectStandardError = false;

  /// Indicates whether input shall be read from a pipe.
  bool RedirectStandardInput = false;

  /// Indicates whether output shall be written to a pipe.
  bool RedirectStandardOutput = false;

  /// Working directory for the process.
  std::string WorkingDirectory;

  ProcessStartInfo()
  {
  }

  ProcessStartInfo(const PathName & fileName) :
    FileName(fileName.ToString())
  {
  }

};

/// Process class.
class MIKTEXNOVTABLE Process
{
public:
  virtual MIKTEXTHISCALL ~Process() = 0;

public:
  virtual FILE * MIKTEXTHISCALL get_StandardInput() = 0;

public:
  /// Gets the redirected output stream.
  /// @return Returns the redirected output stream. Returns 0, if
  /// the output was not redirected.
  virtual FILE * MIKTEXTHISCALL get_StandardOutput() = 0;

public:
  virtual FILE * MIKTEXTHISCALL get_StandardError() = 0;

  /// Waits for the process.
public:
  virtual void MIKTEXTHISCALL WaitForExit() = 0;

  /// Waits for the process.
public:
  virtual bool MIKTEXTHISCALL WaitForExit(int milliseconds) = 0;

  /// Gets the exit code of the process.
  /// The process must have exited.
public:
  virtual int MIKTEXTHISCALL get_ExitCode() const = 0;

public:
  virtual void MIKTEXTHISCALL Close() = 0;

  /// Start the system shell to execute a command.
  /// @param lpszCommandLine Command to be executed.
public:
  static MIKTEXCORECEEAPI(void) StartSystemCommand(const std::string & commandLine);

  /// Executes the system shell to execute a command.
  /// @param lpszCommandLine Command to be executed.
  /// @return Returns true, if the command exited successfully.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string & commandLine);

  /// Executes the system shell to execute a command.
  /// @param lpszCommandLine Command to be executed.
  /// @param pExitCode To be filled with the exit code of the command.
  /// @return Returns true, if the process exited successfully, or
  /// if pExitCode isn't null.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string & commandLine, int * pExitCode);

  /// Executes the system shell to execute a command.
  /// @param lpszCommandLine Command to be executed.
  /// @param pExitCode To be filled with the exit code of the command.
  /// @param callback Callback interface.
  /// @param lpszWorkingDirectory Working directory for the command.
  /// @return Returns true, if the process exited successfully, or
  /// if pExitCode isn't null.
public:
  static MIKTEXCORECEEAPI(bool) ExecuteSystemCommand(const std::string & commandLine, int * pExitCode, IRunProcessCallback * callback, const char * lpszWorkingDirectory);

  /// Executes a process.
  /// @param lpszFileName The name of an executable file to run in the process.
  /// @param lpszArguments The command-line arguments to pass when starting
  /// the process.
public:
  static MIKTEXCORECEEAPI(void) Run(const PathName & fileName, const std::string & arguments);

public:
  static void Run(const PathName & fileName)
  {
    Run(fileName, "");
  }

public:
  static MIKTEXCORECEEAPI(void) Run(const PathName & fileName, const std::string & arguments, IRunProcessCallback * callback);

public:
  static MIKTEXCORECEEAPI(bool) Run(const PathName & fileName, const std::string & arguments, IRunProcessCallback * callback, int * pExitCode, const char * lpszWorkingDirectory);

public:
  static MIKTEXCORECEEAPI(Process *) Start(const ProcessStartInfo & startinfo);

public:
  static MIKTEXCORECEEAPI(void) Start(const PathName & fileName, const std::string & arguments, FILE * pFileStandardInput, FILE ** ppFileStandardInput, FILE ** ppFileStandardOutput, FILE ** ppFileStandardError, const char * lpszWorkingDirectory);

public:
  static void Start(const PathName & fileName)
  {
    Start(fileName, "", nullptr, nullptr, nullptr, nullptr, nullptr);
  }

public:
  static void Start(const PathName & fileName, const std::string & arguments)
  {
    Start(fileName, arguments, nullptr, nullptr, nullptr, nullptr, nullptr);
  }

};

class MIKTEXNOVTABLE Process2 :
  public Process
{
public:
  MIKTEXTHISCALL ~Process2() override = 0;

public:
  virtual Process2 * MIKTEXTHISCALL get_Parent() = 0;

public:
  virtual std::string MIKTEXTHISCALL get_ProcessName() = 0;

public:
  static MIKTEXCORECEEAPI(Process2 *) GetCurrentProcess();

public:
  static MIKTEXCORECEEAPI(std::vector<std::string>) GetInvokerNames();
};

MIKTEX_CORE_END_NAMESPACE;

#endif
