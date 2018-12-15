/* Lockfile.cpp: memory mapped files

   Copyright (C) 2018 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <chrono>
#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "miktex/Core/LockFile.h"
#include "miktex/Core/Process.h"
#include "miktex/Core/StreamReader.h"
#include "miktex/Core/StreamWriter.h"

using namespace MiKTeX::Core;

using namespace std;
using namespace chrono_literals;

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
    } catch (const exception&)
    {
    }
  }
public:
  LockFileImpl(const PathName& path) :
    path(path)
  {
  }
public:
  bool MIKTEXTHISCALL Lock(chrono::milliseconds timeout) override;
public:
  void MIKTEXTHISCALL Unlock() override;
private:
  bool IsGarbage();
private:
  PathName path;
private:
  bool locked = false;
};

LockFile::~LockFile() noexcept
{
}

unique_ptr<LockFile> LockFile::Create(const PathName& path)
{
  return make_unique<LockFileImpl>(path);
}

bool LockFileImpl::Lock(chrono::milliseconds timeout)
{
  if (locked)
  {
    MIKTEX_UNEXPECTED();
  }
  chrono::time_point<chrono::high_resolution_clock> tryUntil = chrono::high_resolution_clock::now() + timeout;
  do
  {
    try
    {
      unique_ptr<FILE, decltype(&fclose)> file(File::Open(path, FileMode::CreateNew, FileAccess::Write), fclose);
      fputs(fmt::format("{}\n", Process::GetCurrentProcess()->GetSystemId()).c_str(), file.get());
      fputs(fmt::format("{}\n", Process::GetCurrentProcess()->get_ProcessName()).c_str(), file.get());
      locked = true;
    }
    catch (const FileExistsException&)
    {
    }
    if (!locked)
    {
      if (IsGarbage())
      {
        File::Delete(path);
      }
      else
      {
        this_thread::sleep_for(10ms);
      }
    }
  } while (!locked && chrono::high_resolution_clock::now() < tryUntil);
  return locked;
}

void MIKTEXTHISCALL LockFileImpl::Unlock()
{
  if (!locked)
  {
    MIKTEX_UNEXPECTED();
  }
  locked = false;
  File::Delete(path);
}

bool LockFileImpl::IsGarbage()
{
  string pid;
  string processName;
  try
  {
    StreamReader reader(path);
    reader.ReadLine(pid);
    reader.ReadLine(processName);
  }
  catch (const IOException&)
  {
    return false;
  }
  unique_ptr<Process> p = Process::GetProcess(std::stoi(pid));
  return p == nullptr || p->get_ProcessName() != processName;
}
