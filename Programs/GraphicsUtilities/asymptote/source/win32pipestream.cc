/**
 * @file win32pipestream.cc
 * @brief A reimplementation of pipestream for win32
 * @author Supakorn "Jamie" Rassameemasmuang (jamievlin [at] outlook.com)
 */
#if defined(_WIN32)
#include "win32pipestream.h"
#include "util.h"
#include "errormsg.h"

namespace w32
{

namespace cw32 = camp::w32;

Win32IoPipeStream::~Win32IoPipeStream()
{
  // this is not the best idea, but this is what the
  // original code looks like (pipestream.h)
  pipeclose();
}

void Win32IoPipeStream::open(
        mem::vector<string> const& command, char const* hint,
        char const* application, int out_fileno)
{
  if (out_fileno != STDOUT_FILENO && out_fileno != STDERR_FILENO)
  {
    camp::reportError("out_fileno must be stdout or stdin");
  }

  // creating pipes
  SECURITY_ATTRIBUTES pipeSecurityAttr = {};
  pipeSecurityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  pipeSecurityAttr.bInheritHandle = true;
  pipeSecurityAttr.lpSecurityDescriptor = nullptr;

  {
    cw32::HandleRaiiWrapper processStdinRd;
    cw32::checkResult(CreatePipe(processStdinRd.put(), &processStdinWr, &pipeSecurityAttr, 0));

    cw32::HandleRaiiWrapper processOutWr;
    cw32::checkResult(CreatePipe(&processOutRd, processOutWr.put(), &pipeSecurityAttr, 0));

    // building command
    string cmd= camp::w32::buildWindowsCmd(command);

    STARTUPINFOA startInfo= {};
    startInfo.cb= sizeof(startInfo);
    startInfo.dwFlags= STARTF_USESTDHANDLES;
    startInfo.hStdInput= processStdinRd.getHandle();
    startInfo.hStdOutput= out_fileno == STDOUT_FILENO ? processOutWr.getHandle() : GetStdHandle(STD_OUTPUT_HANDLE);
    startInfo.hStdError= out_fileno == STDERR_FILENO ? processOutWr.getHandle() : GetStdHandle(STD_ERROR_HANDLE);

    auto const result= CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr, nullptr, true,
            0,
            nullptr, nullptr,
            &startInfo,
            &procInfo);
    if (!result)
    {
      execError(command.at(0).c_str(), hint, application);
      cw32::checkResult(result, "Cannot open application");
    }
  }

  ZeroMemory(buffer, BUFFER_LEN);
  Running=true;
  pipeopen=true;
  pipein=true;
  block(false,true);
}

void Win32IoPipeStream::block(bool write, bool read)
{
  /*
   * Side note: what the hell, microsoft?
   *
   * Why is setting mode for anonymous pipe also called
   * "SetNamedPipeHandleState", why not "SetPipeHandleState"?
   *
   * (see https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-setnamedpipehandlestate?redirectedfrom=MSDN)
   */

  DWORD blockMode = PIPE_WAIT;
  DWORD noBlockMode = PIPE_NOWAIT;

  cw32::checkResult(SetNamedPipeHandleState(
          processStdinWr,
          write ? &blockMode : &noBlockMode,
          nullptr, nullptr
          ));

  cw32::checkResult(SetNamedPipeHandleState(
          processOutRd,
          read ? &blockMode : &noBlockMode,
          nullptr, nullptr
          ));
}

void Win32IoPipeStream::eof()
{
  if(pipeopen && pipein) {
    camp::w32::checkResult(CloseHandle(&processStdinWr));
    pipein=false;
  }
}

void Win32IoPipeStream::pipeclose()
{
  if(pipeopen) {
    cw32::checkResult(CloseHandle(processOutRd));

    if (procInfo.hProcess != nullptr)
    {
      if (!TerminateProcess(procInfo.hProcess, 0) && GetLastError() != ERROR_ACCESS_DENIED)
      {
        camp::reportError("cannot terminate process");
      }
    }
    Running=false;
    pipeopen=false;
    wait();
  }
}

bool Win32IoPipeStream::isopen() const
{
  return pipeopen;
}

Win32IoPipeStream::Win32IoPipeStream(
        mem::vector<string> const& command, char const* hint, char const* application, int out_fileno)
{
  open(command, hint, application, out_fileno);
}

int Win32IoPipeStream::wait()
{
  DWORD retcode = 0;
  if (procInfo.hProcess == nullptr)
  {
    return static_cast<int>(retcode);
  }

  switch(WaitForSingleObject(procInfo.hProcess, INFINITE))
  {
    case WAIT_OBJECT_0:
      cw32::checkResult(GetExitCodeProcess(procInfo.hProcess, &retcode));
      break;
    default:
      closeProcessHandles();
      camp::reportError("Wait for process error");
      break;
  }

  closeProcessHandles();
  return static_cast<int>(retcode);
}

void Win32IoPipeStream::Write(string const& s)
{
  if (!pipeopen)
  {
    return;
  }
  if(settings::verbose > 2) cerr << s;

  DWORD bytesWritten=0;

  cw32::checkResult(WriteFile(
          processStdinWr,
          s.c_str(), s.length(),
          &bytesWritten, nullptr
          ));

  if (static_cast<DWORD>(s.length()) != bytesWritten)
  {
    camp::reportFatal("write to pipe failed");
  }

}

void Win32IoPipeStream::wait(char const* prompt)
{
  sbuffer.clear();
  size_t plen=strlen(prompt);

  do {
    readbuffer();
    if(*buffer == 0) camp::reportError(sbuffer);
    sbuffer.append(buffer);

    if(tailequals(sbuffer.c_str(),sbuffer.size(),prompt,plen)) break;
  } while(true);
}

ssize_t Win32IoPipeStream::readbuffer()
{
  if (!(Running && pipeopen))
  {
    return 0;
  }

  DWORD nc;

  if (!ReadFile(processOutRd, buffer, BUFFER_LEN - 1, &nc, nullptr))
  {
    if (GetLastError() != ERROR_BROKEN_PIPE)
    {
      // process could have exited
      camp::reportError("read failed from pipe");
    }
  }

  buffer[nc]=0;

  if (nc > 0)
  {
    if (settings::verbose > 2)
    {
      cerr << buffer;
    }
  }
  else if (procInfo.hProcess != nullptr)
  {
    switch (WaitForSingleObject(procInfo.hProcess, 0))
    {
      case WAIT_OBJECT_0:
      {
        closeProcessHandles();
        Running=false;
        break;
      }
      case WAIT_TIMEOUT:
        break;
      default:
        camp::reportError("Waiting for process failed");
        break;
    }
  }
  return nc;
}

string Win32IoPipeStream::readline()
{
  sbuffer.clear();
  int nc;
  do {
    nc=readbuffer();
    sbuffer.append(buffer);
  } while(buffer[nc-1] != '\n' && Running);
  return sbuffer;
}

bool Win32IoPipeStream::tailequals(char const* buf, size_t len, char const* prompt, size_t plen)
{
  const char *a=buf+len;
  const char *b=prompt+plen;
  while(b >= prompt) {
    if(a < buf) return false;
    if(*a != *b) return false;
    // Handle MSDOS incompatibility:
    if(a > buf && *a == '\n' && *(a-1) == '\r') --a;
    --a; --b;
  }
  return true;
}

#pragma region "private functions"
void Win32IoPipeStream::closeProcessHandles()
{
  if (procInfo.hProcess != nullptr)
  {
    cw32::checkResult(CloseHandle(procInfo.hProcess));
    procInfo.hProcess=nullptr;
  }

  if (procInfo.hThread != nullptr)
  {
    cw32::checkResult(CloseHandle(procInfo.hThread));
    procInfo.hThread=nullptr;
  }
}

Win32IoPipeStream& Win32IoPipeStream::operator<< (imanip func)
{
  return (*func)(*this);
}

Win32IoPipeStream& Win32IoPipeStream::operator>>(string& s)
{
  readbuffer();
  s=buffer;
  return *this;
}

Win32IoPipeStream& Win32IoPipeStream::operator<<(const string& s)
{
  Write(s);
  return *this;
}
string Win32IoPipeStream::getbuffer()
{
  return sbuffer;
}

bool Win32IoPipeStream::running()
{
  return Running;
}

#pragma endregion

}
#endif
