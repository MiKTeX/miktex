/* miktex/PipeStream.cpp:

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

#if defined(MIKTEX_WINDOWS)
#include <io.h>
#endif

#include <miktex/App/Application>

#include <miktex/Core/FileStream>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>

#include "PipeStream.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

using namespace MiKTeX::Aymptote;

PipeStream::~PipeStream()
{
  Close();
}

void PipeStream::Open(const PathName& fileName, const vector<string>& arguments)
{
  Application::GetApplication()->LogInfo("starting PipeStream child process: " + StringUtil::Flatten(arguments, ' '));
  childStartInfo.FileName = fileName.ToString();
  childStartInfo.Arguments = arguments;
  childStartInfo.RedirectStandardInput = true;
  childStartInfo.RedirectStandardError = true;
  childStartInfo.RedirectStandardOutput = true;
  childProcess = Process::Start(childStartInfo);
  childStdinFile = childProcess->get_StandardInput();
  StartThreads();
}

void PipeStream::Close()
{
  CloseIn();
  StopThreads();
  if (childProcess != nullptr)
  {
    if (!childProcess->WaitForExit(1000))
    {
      Application::GetApplication()->LogWarn("PipeStream child process still running: " + StringUtil::Flatten(childStartInfo.Arguments, ' '));
    }
  }
}

void PipeStream::CloseIn()
{
  if (childStdinFile != nullptr)
  {
    if (fclose(childStdinFile) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fclose");
    }
    childStdinFile = nullptr;
  }
}

void PipeStream::Write(const void* buf, size_t size)
{
  if (IsUnsuccessful())
  {
    throw childStdoutReaderThreadException;
  }
  if (!IsChildRunning())
  {
    MIKTEX_FATAL_ERROR_2("Broken pipe", "argv0", childStartInfo.FileName);
  }
  if (fwrite(buf, 1, size, childStdinFile) != size)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
  if (fflush(childStdinFile) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fflush");
  }
}

size_t PipeStream::Read(void* buf, size_t size)
{
  if (IsUnsuccessful())
  {
    throw childStdoutReaderThreadException;
  }
  return childStdoutPipe.Read(buf, size);
}

int PipeStream::Wait()
{
  if (!childProcess->WaitForExit(10000))
  {
    MIKTEX_FATAL_ERROR("PipeStream child process did not complete");
  }
  return childProcess->get_ExitCode();
}

void PipeStream::StartThreads()
{
  childStdoutReaderThread = std::thread(&PipeStream::ChildStdoutReaderThread, this);
}

void PipeStream::StopThreads()
{
  if (childStdoutReaderThread.joinable())
  {
    childStdoutPipe.Done();
    childStdoutReaderThread.join();
  }
}

void PipeStream::ChildStdoutReaderThread()
{
  try
  {
    FileStream childStdoutFile(childProcess->get_StandardOutput());
    HANDLE childStdoutFileHandle = (HANDLE)_get_osfhandle(fileno(childStdoutFile.Get()));
    if (childStdoutFileHandle == INVALID_HANDLE_VALUE)
    {
      MIKTEX_UNEXPECTED();
    }
    const size_t BUFFER_SIZE = 512;
    unsigned char inbuf[BUFFER_SIZE];
    do
    {
      DWORD avail;
      if (!PeekNamedPipe(childStdoutFileHandle, nullptr, 0, nullptr, &avail, nullptr))
      {
        if (GetLastError() == ERROR_BROKEN_PIPE)
        {
          Application::GetApplication()->LogWarn("broken PipeStream after " + std::to_string(childStdoutTotalBytes) + " bytes");
          break;
        }
        MIKTEX_FATAL_WINDOWS_ERROR("PeekNamedPipe");
      }
      if (avail == 0)
      {
        Sleep(1);
        continue;
      }
      size_t n = childStdoutFile.Read(inbuf, BUFFER_SIZE > avail ? avail : BUFFER_SIZE);
      childStdoutPipe.Write(inbuf, n);
      childStdoutTotalBytes += n;
    }
    while (true);
    Finish(true);
    childStdoutFile.Close();
    childStdoutPipe.Done();
  }
  catch (const MiKTeX::Core::MiKTeXException& e)
  {
    childStdoutReaderThreadException = e;
    Finish(false);
    childStdoutPipe.Done();
    Application::GetApplication()->LogError("MiKTeX exception caught: "s + e.what());
  }
  catch (const std::exception& e)
  {
    childStdoutReaderThreadException = MiKTeX::Core::MiKTeXException(e.what());
    Finish(false);
    childStdoutPipe.Done();
    Application::GetApplication()->LogError("std exception caught: "s + e.what());
  }
}
