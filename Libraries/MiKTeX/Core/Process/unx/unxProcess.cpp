/* unxProcess.cpp:

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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(__APPLE__)
#  include <libproc.h>
#  include <sys/proc.h>
#endif

#if defined(__FreeBSD__)
#   include <sys/param.h>
#   include <sys/queue.h>
#   include <sys/socket.h>
#   include <sys/sysctl.h>
#   include <sys/user.h>
#   include <kvm.h>
#   include <libprocstat.h>
#   include <fcntl.h>
#endif

#include <thread>

#include <miktex/Core/Directory>
#include <miktex/Core/Environment>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/StreamReader>

#include <miktex/Util/PathNameUtil>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "unxProcess.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

const int filenoStdin = 0;
const int filenoStdout = 1;
const int filenoStderr = 2;

#if defined(HAVE_FORK)
#  define FORK fork
#else
#  error this system does not have a working fork() function
#endif

#if defined(HAVE_VFORK)
#  define VFORK vfork
#else
#  define VFORK fork
#endif

MIKTEXSTATICFUNC(int) Dup(int fd)
{
  int dupfd = dup(fd);
  if (dupfd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("dup");
  }
  return dupfd;
}

MIKTEXSTATICFUNC(void) Dup2(int fd, int fd2)
{
  if (dup2(fd, fd2) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("dup2");
  }
}

MIKTEXSTATICFUNC(void) Close_(int fd)
{
  if (close(fd) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("close");
  }
}

class Pipe
{
public:
  ~Pipe() noexcept
  {
    try
    {
      Dispose();
    }
    catch (const exception&)
    {
    }
  }

public:
  void Create()
  {
    if (pipe(twofd) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("pipe");
    }
  }

public:
  int GetReadEnd() const
  {
    return twofd[0];
  }

public:
  int GetWriteEnd() const
  {
    return twofd[1];
  }

public:
  int StealReadEnd()
  {
    int fd = twofd[0];
    twofd[0] = -1;
    return fd;
  }

public:
  int StealWriteEnd()
  {
    int fd = twofd[1];
    twofd[1] = -1;
    return fd;
  }

public:
  void CloseReadEnd()
  {
    int fd = twofd[0];
    twofd[0] = -1;
    Close_(fd);
  }

public:
  void CloseWriteEnd()
  {
    int fd = twofd[1];
    twofd[1] = -1;
    Close_(fd);
  }

public:
  void Dispose()
  {
    if (twofd[0] >= 0)
    {
      CloseReadEnd();
    }
    if (twofd[1] >= 0)
    {
      CloseWriteEnd();
    }
  }

private:
  int twofd[2] = { -1, -1 };
};

unique_ptr<Process> Process::Start(const ProcessStartInfo& startinfo)
{
  return make_unique<unxProcess>(startinfo);
}

#if defined(NDEBUG)
#  define TRACEREDIR 0
#else
#  define TRACEREDIR 1
#endif

void unxProcess::Create()
{
  MIKTEX_EXPECT(!startinfo.FileName.empty());

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();

  PathName fileName;

  if (PathNameUtil::IsAbsolutePath(startinfo.FileName) || session == nullptr || !session->FindFile(startinfo.FileName, FileType::EXE, fileName))
  {
    fileName = startinfo.FileName;
  }

  Argv argv(startinfo.Arguments.empty() ? vector<string>{ PathName(startinfo.FileName).GetFileNameWithoutExtension().ToString() } : startinfo.Arguments);

  Pipe pipeStdout;
  Pipe pipeStderr;
  Pipe pipeStdin;

  int fdChildStdin = -1;
  int fdChildStderr = -1;

  // create stdout pipe
  if (startinfo.RedirectStandardOutput)
  {
    pipeStdout.Create();
#if 1
    if (!startinfo.RedirectStandardError)
    {
      fdChildStderr = Dup(pipeStdout.GetWriteEnd());
    }
#endif
  }

  // create stderr pipe
  if (startinfo.RedirectStandardError)
  {
    pipeStderr.Create();
  }

  // use file descriptor or create stdin pipe
  if (startinfo.StandardInput != nullptr)
  {
    // caller feeds stdin via a stdio stream
    fdChildStdin = fileno(startinfo.StandardInput);
    if (fdChildStdin < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fileno");
    }
  }
  else if (startinfo.RedirectStandardInput)
  {
    pipeStdin.Create();
  }

  if (session != nullptr)
  {
    session->UnloadFilenameDatabase();
  }

  tmpFile = TemporaryFile::Create();
  tmpEnv.Set(MIKTEX_ENV_EXCEPTION_PATH, tmpFile->GetPathName().ToString());

  // fork
  if (session != nullptr)
  {
    session->trace_process->WriteLine("core", TraceLevel::Info, "forking...");
  }
  if (pipeStdout.GetReadEnd() >= 0
    || pipeStderr.GetReadEnd() >= 0
    || pipeStdin.GetReadEnd() >= 0
    || fdChildStdin >= 0
    || fdChildStderr >= 0)
  {
    pid = FORK();
  }
  else
  {
    pid = VFORK();
  }
  if (pid < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fork");
  }

  if (pid == 0)
  {
    try
    {
      // I'm a child
      if (pipeStdout.GetWriteEnd() >= 0)
      {
        Dup2(pipeStdout.GetWriteEnd(), filenoStdout);
      }
      if (pipeStderr.GetWriteEnd() >= 0)
      {
        Dup2(pipeStderr.GetWriteEnd(), filenoStderr);
      }
      else if (fdChildStderr >= 0)
      {
        Dup2(fdChildStderr, filenoStderr);
        ::Close_(fdChildStderr);
      }
      if (pipeStdin.GetReadEnd() >= 0)
      {
        Dup2(pipeStdin.GetReadEnd(), filenoStdin);
      }
      else if (fdChildStdin >= 0)
      {
        Dup2(fdChildStdin, filenoStdin);
        ::Close_(fdChildStdin);
      }
      pipeStdout.Dispose();
      pipeStderr.Dispose();
      pipeStdin.Dispose();
      if (session != nullptr)
      {
        session->SetEnvironmentVariables();
      }
      if (!startinfo.WorkingDirectory.empty())
      {
        Directory::SetCurrent(PathName(startinfo.WorkingDirectory));
      }
      if (startinfo.Daemonize)
      {
        if (setsid() == -1)
        {
          MIKTEX_FATAL_CRT_ERROR("setsid");
        }
      }
      if (session != nullptr)
      {
        string args;
        for (int idx = 0; argv[idx] != nullptr; ++idx)
        {
          args += fmt::format("\"{0}\"", argv[idx]);
          if (argv[idx + 1] != nullptr)
          {
            args += ", ";
          }
        }
        session->trace_process->WriteLine("core", TraceLevel::Info, fmt::format("execv: \"{0}\", [ {1} ]", fileName, args));
      }
      execv(fileName.GetData(), const_cast<char*const*>(argv.GetArgv()));
      perror("execv failed");
    }
    catch (const exception&)
    {
    }
    _exit(127);
  }

  MIKTEX_ASSERT(pid > 0);

  if (startinfo.RedirectStandardOutput)
  {
    fdStandardOutput = pipeStdout.StealReadEnd();
  }

  if (startinfo.RedirectStandardError)
  {
    fdStandardError = pipeStderr.StealReadEnd();
  }

  if (startinfo.RedirectStandardInput)
  {
    fdStandardInput = pipeStdin.StealWriteEnd();
  }

  if (fdChildStderr >= 0)
  {
    ::Close_(fdChildStderr);
  }

  pipeStdout.Dispose();
  pipeStderr.Dispose();
  pipeStdin.Dispose();
}

unxProcess::unxProcess(const ProcessStartInfo& startinfo) :
  startinfo(startinfo)
{
  Create();
}

unxProcess::~unxProcess()
{
  try
  {
    Close();
  }
  catch (const exception&)
  {
  }
}

void unxProcess::Close()
{
  if (fdStandardOutput >= 0)
  {
    close(fdStandardOutput);
    fdStandardOutput = -1;
  }
  if (fdStandardError >= 0)
  {
    close(fdStandardError);
    fdStandardError = -1;
  }
  if (fdStandardInput >= 0)
  {
    close(fdStandardInput);
    fdStandardInput = -1;
  }
  this->pid = -1;
  if (tmpFile != nullptr)
  {
    tmpEnv.Restore();
    tmpFile->Delete();
    tmpFile = nullptr;
  }
}

FILE* unxProcess::get_StandardInput()
{
  if (pFileStandardInput != nullptr)
  {
    return pFileStandardInput;
  }
  if (fdStandardInput < 0)
  {
    return nullptr;
  }
  pFileStandardInput = FdOpen(fdStandardInput, "wb");
  fdStandardInput = -1;
  return pFileStandardInput;
}

FILE* unxProcess::get_StandardOutput()
{
  if (pFileStandardOutput != nullptr)
  {
    return pFileStandardOutput;
  }
  if (fdStandardOutput < 0)
  {
    return nullptr;
  }
  pFileStandardOutput = FdOpen(fdStandardOutput, "rb");
  fdStandardOutput = -1;
  return pFileStandardOutput;
}

FILE* unxProcess::get_StandardError()
{
  if (pFileStandardError != nullptr)
  {
    return pFileStandardError;
  }
  if (fdStandardError < 0)
  {
    return nullptr;
  }
  pFileStandardError = FdOpen(fdStandardError, "rb");
  fdStandardError = -1;
  return pFileStandardError;
}

void unxProcess::WaitForExit()
{
  if (this->pid > 0)
  {
    auto session = SessionImpl::GetSession();
    session->trace_process->WriteLine("core", fmt::format("waiting for process {0}", this->pid));
    pid_t pid = this->pid;
    this->pid = -1;
    while (waitpid(pid, &status, 0) <= 0)
    {
      if (errno != EINTR)
      {
        MIKTEX_FATAL_CRT_ERROR("waitpid");
      }
    }
    if (WIFEXITED(status) != 0)
    {
      session->trace_process->WriteLine("core", fmt::format("process {0} exited with status {1}", pid, WEXITSTATUS(status)));
    }
    else if (WIFSIGNALED(status) != 0)
    {
      session->trace_process->WriteLine("core", fmt::format("process {0} terminated due to signal {1}", pid, WTERMSIG(status)));
    }
    else if (WIFSTOPPED(status) != 0)
    {
      session->trace_process->WriteLine("core", fmt::format("process {0} stopped due to signal {1}", pid, WSTOPSIG(status)));
    }
    else if (WIFCONTINUED(status) != 0)
    {
      session->trace_process->WriteLine("core", fmt::format("process {0} continued", pid));
    }
  }
}

bool unxProcess::WaitForExit(int milliseconds)
{
  if (this->pid <= 0)
  {
    return true;
  }
  do
  {
    pid_t pid = waitpid(this->pid, &status, WNOHANG);
    if (pid == this->pid)
    {
      this->pid = -1;
      return true;
    }
    else if (pid < 0)
    {
      this->pid = -1;
      MIKTEX_FATAL_CRT_ERROR("waitpid");
    }
    MIKTEX_ASSERT(pid == 0);
    this_thread::sleep_for(chrono::milliseconds(1));
    --milliseconds;
  } while (milliseconds > 0);
  return false;
}

ProcessExitStatus unxProcess::get_ExitStatus() const
{
  if (WIFEXITED(status) != 0)
  {
    return ProcessExitStatus::Exited;
  }
  else if (WIFSIGNALED(status) != 0)
  {
    ProcessExitStatus::Signaled;
  }
  else if (WIFSTOPPED(status) != 0)
  {
    ProcessExitStatus::Stopped;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}


int unxProcess::get_ExitCode() const
{
  if (WIFEXITED(status) != 0)
  {
    return WEXITSTATUS(status);
  }
  else if (WIFSIGNALED(status) != 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("Process terminated due to a signal."), "fileName", startinfo.FileName, "signal", std::to_string(WTERMSIG(status)));
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

bool unxProcess::get_Exception(MiKTeXException& ex) const
{
  return MiKTeXException::Load(tmpFile->GetPathName().ToString(), ex);
}

string ConfStr(int name)
{
  size_t n = confstr(name, nullptr, 0);
  if (n == 0)
  {
    MIKTEX_FATAL_CRT_ERROR("confstr");
  }
  CharBuffer<char> result(n);
  n = confstr(name, result.GetData(), n);
  if (n == 0)
  {
    MIKTEX_FATAL_CRT_ERROR("confstr");
  }
  MIKTEX_EXPECT(n <= result.GetCapacity());
  return result.ToString();
}

MIKTEXSTATICFUNC(PathName) FindSystemShell()
{
#if defined(HAVE_CONFSTR) && defined(_CS_SHELL)
  return PathName(ConfStr(_CS_SHELL));
#else
  return PathName("/bin/sh");
#endif
}

MIKTEXSTATICFUNC(vector<string>) Wrap(const string& commandLine)
{
  return vector<string> {
    FindSystemShell().ToString(),
    "-c",
    commandLine
  };
}

unique_ptr<Process> Process::StartSystemCommand(const string& commandLine, FILE** ppFileStandardInput, FILE** ppFileStandardOutput)
{
  vector<string> arguments = Wrap(commandLine);
  ProcessStartInfo startinfo;
  startinfo.FileName = arguments[0];
  startinfo.Arguments = arguments;
  startinfo.RedirectStandardInput = ppFileStandardInput != nullptr;
  startinfo.RedirectStandardOutput = ppFileStandardOutput != nullptr;
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

bool Process::ExecuteSystemCommand(const string& commandLine, int* exitCode, IRunProcessCallback* callback, const char* directory)
{
  vector<string> arguments = Wrap(commandLine);
  return Process::Run(PathName(arguments[0]), arguments, callback, exitCode, directory);
}

unique_ptr<Process> Process::GetCurrentProcess()
{
  unique_ptr<unxProcess> currentProcess = make_unique<unxProcess>();
  currentProcess->pid = getpid();
  return currentProcess;
}

unique_ptr<Process> Process::GetProcess(int systemId)
{
  if (kill(systemId, 0) != 0)
  {
    if (errno == ESRCH)
    {
      return nullptr;
    }
    MIKTEX_FATAL_CRT_ERROR("kill");
  }
  unique_ptr<unxProcess> process = make_unique<unxProcess>();
  process->pid = systemId;
  return process;
}

unique_ptr<Process> unxProcess::get_Parent()
{
  ProcessInfo processInfo = GetProcessInfo();
  unique_ptr<unxProcess> parentProcess = make_unique<unxProcess>();
  parentProcess->pid = processInfo.parent;
  return parentProcess;
}

string unxProcess::get_ProcessName()
{
#if defined(__linux__)
  PathName path("/proc");
  path /= std::to_string(pid);
  path /= "comm";
  if (!File::Exists(path))
  {
    // process does not exist anymore
    return "";
  }
  StreamReader reader(path);
  string line;
  while (reader.ReadLine(line))
  {
    return line;
  }
  MIKTEX_UNEXPECTED();
#elif defined(__APPLE__)
  char path[PROC_PIDPATHINFO_MAXSIZE];
  if (proc_pidpath(pid, path, sizeof(path)) == 0)
  {
    if (errno == ESRCH)
    {
      return "";
    }
    MIKTEX_FATAL_CRT_ERROR("proc_pidpath")
  }
  return PathName(path).GetFileName().ToString();
#elif defined(__FreeBSD__)
  std::string nameFromProcstat;
  kvm_t *kvm = kvm_open(nullptr, "/dev/null", nullptr, O_RDONLY, "");
  if (kvm)
  {
    int cnt;
    struct kinfo_proc *kp = kvm_getprocs(kvm, KERN_PROC_PID, pid, &cnt);
    if (kp)
    {
      struct procstat *ps = procstat_open_sysctl();
      char **argv = procstat_getargv(ps, kp, 0);
      if (argv != nullptr && argv[0] != nullptr)
        nameFromProcstat = std::string(argv[0]);
      procstat_close(ps);
    }
    kvm_close(kvm);
  }

  if (!nameFromProcstat.empty())
  {
    return std::string(std::find_if(nameFromProcstat.rbegin(), nameFromProcstat.rend(),
            [](char c) {return c == '\\' || c == '/'; })
        .base(), nameFromProcstat.end());
  }

  struct kinfo_proc kp;
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
  size_t len = sizeof(kp);
  u_int mib_len = sizeof(mib)/sizeof(u_int);

  if (sysctl(mib, mib_len, &kp, &len, NULL, 0) < 0)
    MIKTEX_FATAL_CRT_ERROR("get_ProcessName:sysctl");

  if (kp.ki_pid != pid)
    MIKTEX_FATAL_CRT_ERROR("get_ProcessName: ki_pid != pid");

  return std::string(kp.ki_comm);

#else
#error Unimplemented: unxProcess::get_ProcessName()
#endif
}

int unxProcess::GetSystemId()
{
  return this->pid;
}

ProcessInfo unxProcess::GetProcessInfo()
{
  ProcessInfo processInfo;
  processInfo.name = get_ProcessName();
#if defined(__linux__)
  PathName path("/proc");
  path /= std::to_string(pid);
  path /= "stat";
  if (!File::Exists(path))
  {
    // process does not exist anymore
    return processInfo;
  }
  StreamReader reader(path);
  string line;
  while (reader.ReadLine(line))
  {
    Tokenizer tok(line, " ");
    MIKTEX_ASSERT(std::stoi(*tok) == pid);
    ++tok;
    ++tok;
    string state = *tok;
    MIKTEX_ASSERT(state.length() == 1);
    switch (state[0])
    {
      case 'R':
        processInfo.status = ProcessStatus::Runnable;
        break;
      case 'S':
      case 'D':
        processInfo.status = ProcessStatus::Sleeping;
        break;
      case 'T':
        processInfo.status = ProcessStatus::Stoped;
        break;
      case 'Z':
        processInfo.status = ProcessStatus::Zombie;
        break;
      default:
        processInfo.status = ProcessStatus::Other;
        break;
    }
    ++tok;
    processInfo.parent = std::stoi(*tok);
  }
#elif defined(__APPLE__)
  struct proc_bsdinfo pbi;
  if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &pbi, PROC_PIDTBSDINFO_SIZE) == 0)
  {
    if (errno == ESRCH)
    {
      return processInfo;
    }
    MIKTEX_FATAL_CRT_ERROR("proc_pidpath")
  }
  switch (pbi.pbi_status)
  {
    case SRUN:
      processInfo.status = ProcessStatus::Runnable;
      break;
    case SSLEEP:
      processInfo.status = ProcessStatus::Sleeping;
      break;
    case SSTOP:
      processInfo.status = ProcessStatus::Stoped;
      break;
    case SZOMB:
      processInfo.status = ProcessStatus::Zombie;
      break;
    default:
      processInfo.status = ProcessStatus::Other;
      break;
  }
  processInfo.parent = pbi.pbi_ppid;
#elif defined(__FreeBSD__)
  processInfo.parent = getppid();

  kvm_t *kvm = kvm_open(nullptr, "/dev/null", nullptr, O_RDONLY, "");
  if (kvm)
  {
    int cnt;
    struct kinfo_proc *kp = kvm_getprocs(kvm, KERN_PROC_PID, pid, &cnt);
    if (kp)
    {
      switch (kp->ki_stat)
      {
        case SRUN:
          processInfo.status = ProcessStatus::Runnable;
          break;
        case SSLEEP:
          processInfo.status = ProcessStatus::Sleeping;
          break;
        case SSTOP:
          processInfo.status = ProcessStatus::Stoped;
          break;
        case SZOMB:
          processInfo.status = ProcessStatus::Zombie;
          break;
        default:
          processInfo.status = ProcessStatus::Other;
          break;
      }
    }
    kvm_close(kvm);
  }
#else
#error Unimplemented: unxProcess::GetProcessInfo()
#endif
  return processInfo;
}


void Process::Overlay(const PathName& fileName, const vector<string>& arguments)
{
  MIKTEX_EXPECT(!fileName.Empty());

  Argv argv(arguments.empty() ? vector<string>{ PathName(fileName).GetFileNameWithoutExtension().ToString() } : arguments);

  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->UnloadFilenameDatabase();
    session->SetEnvironmentVariables();
    session->trace_process->WriteLine("core", TraceLevel::Info, fmt::format("execv: {0}", fileName));
    for (int idx = 0; argv[idx] != nullptr; ++idx)
    {
      session->trace_process->WriteLine("core", TraceLevel::Info, fmt::format(" argv[{0}]: {1}", idx, argv[idx]));
    }
  }

  execv(fileName.GetData(), const_cast<char*const*>(argv.GetArgv()));

  MIKTEX_UNEXPECTED();
}
