/* Lockfile.cpp: memory mapped files

   Copyright (C) 2018-2021 Christian Schenk

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

#include <chrono>
#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/LockFile>
#include <miktex/Core/Process>
#include <miktex/Core/StreamReader>

#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include <miktex/Util/PathName>

#include "internal.h"

using namespace std;
using namespace chrono_literals;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

class LockFileImpl :
  public LockFile
{
public:
  LockFileImpl() = delete;
public:
  LockFileImpl(const LockFileImpl& other) = delete;
public:
  LockFileImpl(LockFileImpl&& other) = delete;
public:
  LockFileImpl& operator=(const LockFileImpl& other) = delete;
public:
  LockFileImpl& operator=(LockFileImpl&& other) = delete;
public:
  ~LockFileImpl() override
  {
    try
    {
      if (locked)
      {
        Unlock();
      }
    }
    catch (const exception&)
    {
    }
  }
public:
  LockFileImpl(const PathName& path) :
    path(path)
  {
    trace_lockfile = TraceStream::Open(MIKTEX_TRACE_LOCKFILE);
  }
public:
  bool MIKTEXTHISCALL TryLock(chrono::milliseconds timeout) override;
public:
  void MIKTEXTHISCALL Unlock() override;
private:
  tuple<int, string> ReadLockFile();
private:
  tuple<bool, int, string> CheckLockFile();
private:
  PathName path;
private:
  bool locked = false;
private:
  unique_ptr<TraceStream> trace_lockfile;
};

LockFile::~LockFile() noexcept
{
}

unique_ptr<MiKTeX::Core::LockFile> LockFile::Create(const PathName& path)
{
  return make_unique<LockFileImpl>(path);
}

bool LockFileImpl::TryLock(chrono::milliseconds timeout)
{
  trace_lockfile->WriteLine("core", fmt::format(T_("trying to create lock file {0}"), Q_(path)));
  if (locked)
  {
    MIKTEX_FATAL_ERROR_2(T_("File is locked: {0}"), "path", path.ToString());
  }
  chrono::time_point<chrono::high_resolution_clock> tryUntil = chrono::high_resolution_clock::now() + timeout;
  bool tryAgain;
  do
  {
    tryAgain = false;
    if (!File::Exists(path))
    {
      try
      {
        ofstream os = File::CreateOutputStream(path);
        os
          << Process::GetCurrentProcess()->GetSystemId() << "\n"
          << Process::GetCurrentProcess()->get_ProcessName() << "\n";
        os.close();
        trace_lockfile->WriteLine("core", fmt::format(T_("lock file {0} successfully created"), Q_(path)));
        locked = true;
      }
      catch (const FileExistsException&)
      {
      }
      catch (const UnauthorizedAccessException&)
      {
      }
    }
    if (!locked)
    {
      bool isGarbage;
      int pid;
      string processName;
      tie(isGarbage, pid, processName) = CheckLockFile();
      if (isGarbage)
      {
        trace_lockfile->WriteLine("core", TraceLevel::Warning, fmt::format(T_("removing lock file {0} created by {1} ({2})"), Q_(path), processName, pid));
        try
        {
          File::Delete(path);
        }
        catch (const FileNotFoundException&)
        {
        }
        catch (const exception&)
        {
          throw;
        }
        tryAgain = true;
      }
      else
      {
        this_thread::sleep_for(10ms);
      }
    }
  } while (!locked && (tryAgain || chrono::high_resolution_clock::now() < tryUntil));
  return locked;
}

void MIKTEXTHISCALL LockFileImpl::Unlock()
{
  trace_lockfile->WriteLine("core", fmt::format(T_("removing lock file {0}"), Q_(path)));
  if (!locked)
  {
    MIKTEX_FATAL_ERROR_2(T_("File is not locked: {0}"), "path", path.ToString());
  }
  locked = false;
  File::Delete(path);
}

tuple<int, string> LockFileImpl::ReadLockFile()
{
  string pid;
  string processName;
  StreamReader reader(path);
  reader.ReadLine(pid);
  reader.ReadLine(processName);
  reader.Close();
  return make_tuple(std::stoi(pid), processName);
}

tuple<bool, int, string> LockFileImpl::CheckLockFile()
{
  int pid;
  string processName;
  try
  {
    tie(pid, processName) = ReadLockFile();
  }
  catch (const UnauthorizedAccessException&)
  {
    trace_lockfile->WriteLine("core", fmt::format(T_("permission denied: {0}"), Q_(path)));
    return make_tuple(false, pid, processName);
  }
  catch (const IOException&)
  {
    trace_lockfile->WriteLine("core", fmt::format(T_("could not read lock file {0}"), Q_(path)));
    return make_tuple(false, pid, processName);
  }
  if (pid == -1)
  {
    // permanently locked
    return make_tuple(false, pid, processName);
  }
  if (pid == Process::GetCurrentProcess()->GetSystemId())
  {
    MIKTEX_UNEXPECTED();
  }
  try
  {
    unique_ptr<Process> p = Process::GetProcess(pid);
    if (p == nullptr)
    {
      trace_lockfile->WriteLine("core", fmt::format(T_("owner of lock file {0} does not exist"), Q_(path)));
      return make_tuple(true, pid, processName);
    }
    if (p->get_ProcessName() != processName)
    {
      trace_lockfile->WriteLine("core", fmt::format(T_("owner process {0} ({1}) of lock file {2} does not exist"), processName, pid, Q_(path)));
      return make_tuple(true, pid, processName);
    }
    if (p->GetProcessInfo().status == ProcessStatus::Zombie)
    {
      trace_lockfile->WriteLine("core", fmt::format(T_("owner process {0} ({1}) of lock file {2} is a zombie"), processName, pid, Q_(path)));
      return make_tuple(true, pid, processName);
    }
  }
  catch (const UnauthorizedAccessException&)
  {
    trace_lockfile->WriteLine("core", fmt::format(T_("permission denied: process {0} ({1})"), processName, pid));
  }
  return make_tuple(false, pid, processName);
}
