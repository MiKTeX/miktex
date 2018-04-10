/* miktex/PipeStream.h:

   Copyright (C) 2017-2018 Christian Schenk

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

#include "asy-first.h"
#include "config.h"

#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <miktex/Core/PathName>
#include <miktex/Core/Process>

#include "InProcPipe.h"

#if !defined(MIKTEX_BEGIN_NS)
#define MIKTEX_BEGIN_NS                         \
namespace MiKTeX {                              \
  namespace Aymptote {
#define MIKTEX_END_NS                           \
  }                                             \
}
#endif

MIKTEX_BEGIN_NS;

class PipeStream
{
public:
  virtual ~PipeStream();

public:
  void Open(const MiKTeX::Core::PathName& fileName, const std::vector<std::string>& arguments);

public:
  void Close();

public:
  void CloseIn();

private:
  void StartThreads();

private:
  void StopThreads();

private:
  void ChildStdoutReaderThread();

private:
  FILE* childStdinFile = nullptr;

public:
  void Write(const void* buf, size_t size);

public:
  size_t Read(void* buf, size_t size);

public:
  int Wait();

public:
  bool IsChildRunning()
  {
    return !childStdoutPipe.IsDone();
  }

private:
  std::unique_ptr<MiKTeX::Core::Process> childProcess;

private:
  std::thread childStdoutReaderThread;

private:
  InProcPipe childStdoutPipe;

protected:
  enum State {
    Ready = 1,
    Successful = 2
  };

protected:
  std::atomic_int state{ 0 };

protected:
  bool IsReady()
  {
    return (state.load() & Ready) != 0;
  }

protected:
  bool IsSuccessful()
  {
    return (state.load() & Successful) != 0;
  }

protected:
  bool IsUnsuccessful()
  {
    return state.load() == Ready;
  }

protected:
  void Finish(bool successful)
  {
    state = Ready | (successful ? Successful : 0);
  }

protected:
  MiKTeX::Core::ProcessStartInfo childStartInfo;

protected:
  std::atomic_size_t childStdoutTotalBytes{ 0 };

protected:
  MiKTeX::Core::MiKTeXException childStdoutReaderThreadException;
};

MIKTEX_END_NS;
