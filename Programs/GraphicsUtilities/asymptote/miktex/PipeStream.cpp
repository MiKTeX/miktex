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

#include <io.h>

#include <miktex/Core/FileStream>
#include <miktex/Core/Session>

#include "PipeStream.h"

using namespace MiKTeX::Core;
using namespace std;

using namespace MiKTeX::Aymptote;

PipeStream::~PipeStream()
{
  Close();
}

void PipeStream::Open(const PathName& fileName, const vector<string>& arguments)
{
  ProcessStartInfo startInfo;
  startInfo.FileName = fileName.ToString();
  startInfo.Arguments = arguments;
  startInfo.RedirectStandardInput = true;
  startInfo.RedirectStandardError = true;
  startInfo.RedirectStandardOutput = true;
  process = Process::Start(startInfo);
  inFile = process->get_StandardInput();
  StartThreads();
}

void PipeStream::Close()
{
  CloseIn();
  StopThreads();
  if (process != nullptr)
  {
    process->WaitForExit(1000);
  }
}

void PipeStream::CloseIn()
{
  if (inFile != nullptr)
  {
    fclose(inFile);
    inFile = nullptr;
  }
}

void PipeStream::Write(const void* buf, size_t size)
{
  if (fwrite(buf, 1, size, inFile) != size)
  {
    MIKTEX_FATAL_CRT_ERROR("fwrite");
  }
  fflush(inFile);
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
  process->WaitForExit();
  return process->get_ExitCode();
}

void PipeStream::StartThreads()
{
  childStdoutReaderThread = std::thread(&PipeStream::ChildStdoutReaderThread, this);
}

void PipeStream::StopThreads()
{
  if (childStdoutReaderThread.joinable())
  {
    childStdoutPipe.Close();
    childStdoutReaderThread.join();
  }
}

void PipeStream::ChildStdoutReaderThread()
{
  try
  {
    FileStream outFile(process->get_StandardOutput());
    HANDLE outFileHandle = (HANDLE)_get_osfhandle(fileno(outFile.Get()));
    if (outFileHandle == INVALID_HANDLE_VALUE)
    {
      MIKTEX_UNEXPECTED();
    }
    DWORD mode = PIPE_NOWAIT;
    if (!SetNamedPipeHandleState(outFileHandle, &mode, nullptr, nullptr))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetNamedPipeHandleState");
    }
    const size_t BUFFER_SIZE = 1024 * 16;
    //const size_t BUFFER_SIZE = 1;
    unsigned char inbuf[BUFFER_SIZE];
    do
    {
      size_t n = outFile.Read(inbuf, BUFFER_SIZE);
      if (n == 0)
      {
        break;
      }
      childStdoutPipe.Write(inbuf, n);
    }
    while (true);
    outFile.Close();
    childStdoutPipe.Close();
    Finish(true);
  }
  catch (const MiKTeX::Core::MiKTeXException& e)
  {
    childStdoutReaderThreadException = e;
    Finish(false);
  }
  catch (const std::exception& e)
  {
    childStdoutReaderThreadException = MiKTeX::Core::MiKTeXException(e.what());
    Finish(false);
  }
}
